/*
 * librd - Rapid Development C library
 *
 * Copyright (c) 2012-2013, Magnus Edenhill
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "rd.h"
#include "rdevent.h"
#include "rdiothread.h"
#include "rdlru.h"
#include "rdlog.h"
#include "rdstring.h"

#include <sys/epoll.h>
#include <assert.h>

typedef struct rd_io_hnd_s {
	LIST_ENTRY(rd_io_hnd_s) rioh_link;
	int   rioh_fd;
	int   rioh_refcnt;
	int   rioh_flags;
	int   rioh_events;
	rd_thread_t *rioh_target_thread;
	rd_thread_t *rioh_worker_thread;
	void (*rioh_handler) (int fd, int events, rd_thread_t *target_thread,
			      void *opaque);
	void *rioh_opaque;
} rd_io_hnd_t;

typedef struct rd_io_hnd_event_s {
	rd_io_hnd_t *rioev_rioh;
	int          rioev_events;
} rd_io_hnd_event_t;

/* Maximum number of IO worker threads*/
#define RD_IO_THREAD_WORKERS_MAX   100
/* Maximum waiting time for an event on a thread queue before it picks
 * another thread. */
#define RD_IO_THREAD_WAIT_MAX     10000 /* 10ms */

static LIST_HEAD(, rd_io_hnd_s) rd_io_hnds;
static rd_mutex_t rd_io_hnds_lock;

static int rd_io_thread_fd = -1;
static rd_thread_t *rd_io_thread;
static int rd_io_thread_started;

static rd_lru_t rd_io_worker_lru = RD_LRU_INITIALIZER(rd_io_worker_lru);
static int      rd_io_worker_cnt;

#define rd_io_hnd_keep(rioh)  rd_atomic_add(&(rioh)->rioh_refcnt, 1)

static void rd_io_hnd_destroy (rd_io_hnd_t *rioh) {

	if (rd_atomic_sub(&rioh->rioh_refcnt, 1) > 0)
		return;

	assert(rioh->rioh_fd == -1);

	rd_mutex_lock(&rd_io_hnds_lock);
	LIST_REMOVE(rioh, rioh_link);
	rd_mutex_unlock(&rd_io_hnds_lock);

	free(rioh);
}


static void rd_io_hnd_call (rd_io_hnd_t *rioh, int events) {

	rd_io_hnd_keep(rioh);
	rioh->rioh_handler(rioh->rioh_fd, events,
			   rioh->rioh_target_thread, rioh->rioh_opaque);
	rd_io_hnd_destroy(rioh);

}

static rd_thread_event_f(rd_io_hnd_work) {
	rd_io_hnd_event_t *rioev = ptr;

	rd_io_hnd_call(rioev->rioev_rioh, rioev->rioev_events);
	rd_io_hnd_destroy(rioev->rioev_rioh);

	free(rioev);
}

static void *rd_io_worker_main (void *arg) {

	rd_thread_sigmask(SIG_BLOCK, RD_SIG_ALL, RD_SIG_END);
	
	while (rd_currthread->rdt_state == RD_THREAD_S_RUNNING) {
		rd_thread_poll(RD_POLL_INFINITE);
	}

	return NULL;
}

static rd_thread_t *rd_io_worker_get (void) {
	rd_thread_t *rdt;

	while (1) {
		rd_lru_lock(&rd_io_worker_lru);
		rdt = rd_lru_pop(&rd_io_worker_lru);
		rd_lru_unlock(&rd_io_worker_lru);

		if (!rdt) {
			if (rd_io_worker_cnt >= RD_IO_THREAD_WORKERS_MAX) {
				/* Must not create more workers, wait for
				 * some to become available. */
				rdbg("Out of IO worker threads, waiting...");
				usleep(5000);
				continue;
			}

			/* Create new worker */
			rdt = rd_thread_create(rd_tsprintf("rd:io:%i",
							   rd_io_worker_cnt++),
					       NULL, rd_io_worker_main, NULL);
		}

		return rdt;
	}

}


/**
 * A handle is assigned to a worker thread as to guarantee that all
 * IO operations are performed in order for the same fd.
 * If the currently assigned thread is blocking too long (for another fd)
 * the fd is migrated to a free thread.
 */
static void rd_io_hnd_enqueue_worker (rd_io_hnd_t *rioh, int events) {
	rd_thread_t *rdt;
	rd_io_hnd_event_t *rioev;

	if (rioh->rioh_flags & RD_IO_F_NONBLOCKING) {
		/* The handler has promised us to only perform nonblocking
		 * operations, we can thus do it from the main io thread
		 * instead of a worker thread. */
		rd_io_hnd_call(rioh, events);
		return;
	}

	/* Enqueue event on io worker thread. */
	rioev = malloc(sizeof(*rioev));

	rioev->rioev_rioh   = rioh;
	rioev->rioev_events = events;

	if (!(rdt = rioh->rioh_worker_thread))
		rdt = rd_io_worker_get();

	rd_io_hnd_keep(rioh);
	rd_thread_event_add(rdt, rd_io_hnd_work, rioev);
}


static void *rd_io_thread_main (void *arg) {
	
	rd_thread_sigmask(SIG_BLOCK, RD_SIG_ALL, RD_SIG_END);

	while (rd_currthread->rdt_state == RD_THREAD_S_RUNNING) {
		struct epoll_event events[100];
		int nfds;
		int i;

		nfds = epoll_wait(rd_io_thread_fd,
				  events, RD_ARRAYSIZE(events), -1);
		if (nfds == -1) {
			if (errno == EINTR)
				continue;
			/* FIXME: log */
			rdbg("epoll_wait failed: %s", strerror(errno));
			assert(!*"rd:io epoll_wait failed");
		}

		for (i = 0 ; i < nfds ; i++) {
			struct epoll_event ev = events[i];
			rd_io_hnd_t *rioh = ev.data.ptr;

			if (rioh->rioh_fd == -1)
				continue; /* Handle is decommissioned */

			rd_io_hnd_enqueue_worker(rioh, ev.events);
		}
		

	}
	
	return NULL;
}

static int rd_io_thread_start (void) {

	if ((rd_io_thread_fd = epoll_create(100)) == -1) {
		/* FIXME: log */
		rdbg("Failed to create epoll fd: %s", strerror(errno));
		return -1;
	}

	rd_io_thread = rd_thread_create("rd:io", NULL, rd_io_thread_main, NULL);

	if (!rd_io_thread) {
		/* FIXME: log */
		rdbg("Failed to create rd:io thread: %s", strerror(errno));
		close(rd_io_thread_fd);
		rd_io_thread_fd = -1;
		return -1;
	}

	return 0;
}


static rd_io_hnd_t *rd_io_hnd_get (int fd, int do_create) {
	rd_io_hnd_t *rioh;

	rd_mutex_lock(&rd_io_hnds_lock);
	LIST_FOREACH(rioh, &rd_io_hnds, rioh_link)
		if (rioh->rioh_fd == fd) {
			rd_io_hnd_keep(rioh);
			break;
		}

	if (!rioh && do_create) {
		rioh = calloc(1, sizeof(*rioh));

		rioh->rioh_fd = fd;
		rd_io_hnd_keep(rioh);
		LIST_INSERT_HEAD(&rd_io_hnds, rioh, rioh_link);
	}

	rd_mutex_unlock(&rd_io_hnds_lock);

	return rioh;
}


int rd_io_del (int fd) {
	rd_io_hnd_t *rioh;

	if (!(rioh = rd_io_hnd_get(fd, 0/*no-create*/)))
		return 0;

	if (rioh->rioh_fd != -1) {
		struct epoll_event ev = {};
		int fd = rioh->rioh_fd;
		rioh->rioh_fd = -1;
		epoll_ctl(rd_io_thread_fd, EPOLL_CTL_DEL, fd, &ev);
		rd_io_hnd_destroy(rioh);
	}

	rd_io_hnd_destroy(rioh);

	return 0;
}


int rd_io_add (int fd, int events, int flags, rd_thread_t *target_thread,
	       void (*handler) (int fd, int events,
				rd_thread_t *target_thread, void *opaque),
	       void *opaque) {
	rd_io_hnd_t *rioh;
	struct epoll_event ev;
	int new;

	if (!events)
		return rd_io_del(fd);

	rioh = rd_io_hnd_get(fd, 1/*create*/);
	new = !rioh->rioh_events;

	rioh->rioh_handler = handler;
	rioh->rioh_opaque = opaque;
	rioh->rioh_events = events;
	rioh->rioh_target_thread = target_thread;
	rioh->rioh_flags = flags;

	if (rd_atomic_add(&rd_io_thread_started, 1) == 1) {
		if (rd_io_thread_start() == -1) {
			rd_atomic_sub(&rd_io_thread_started, 1);
			rd_io_hnd_destroy(rioh); /* from .._get() */
			return -1;
		}
	}

	ev.events = events;
	ev.data.ptr = rioh;

	rd_io_hnd_keep(rioh);

	if (epoll_ctl(rd_io_thread_fd, new ? EPOLL_CTL_ADD : EPOLL_CTL_MOD,
		      rioh->rioh_fd, &ev) == -1) {
		/* FIXME: log */
		rdbg("epoll_ctl(%i, %s, fd %i) failed: %s",
		     rd_io_thread_fd, new ? "ADD":"MOD",  rioh->rioh_fd,
		     strerror(errno));
		rd_io_hnd_destroy(rioh); /* from .._get() */
		rd_io_hnd_destroy(rioh); /* pre epoll_ctl */
	}

	return 0;
}
