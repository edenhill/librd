/*
 * librd - Rapid Development C library
 *
 * Copyright (c) 2012, Magnus Edenhill
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
#include "rdthread.h"
#include "rdqueue.h"
#include "rdevent.h"
#include "rdlog.h"

#include <sys/prctl.h>

rd_thread_t *rd_mainthread;
__thread rd_thread_t *rd_currthread;


void rd_thread_init (void) {
	rd_mainthread = calloc(1, sizeof(*rd_mainthread));
	rd_mainthread->rdt_state = RD_THREAD_S_RUNNING;
	rd_mainthread->rdt_name = strdup("main");
	rd_mainthread->rdt_thread = pthread_self();

	rd_fifoq_init(&rd_mainthread->rdt_eventq);

	rd_currthread = rd_mainthread;
}

int rd_thread_poll (int timeout_ms) {
	rd_fifoq_elm_t *rfqe;
	int cnt = 0;
	int nowait = timeout_ms == -1;

	while ((rfqe = rd_fifoq_pop0(&rd_currthread->rdt_eventq,
				     nowait, timeout_ms))) {
		rd_thread_event_t *rte = rfqe->rfqe_ptr;
		
		rd_thread_event_call(rte);
		
		rd_fifoq_elm_release(&rd_currthread->rdt_eventq, rfqe);

		cnt++;
	}

	return cnt;
}

void rd_thread_cleanup (void) {
	extern void rd_string_thread_cleanup ();
	rd_string_thread_cleanup();
}


void rd_thread_dispatch (void) {

	while (rd_currthread->rdt_state == RD_THREAD_S_RUNNING) {
		/* FIXME: Proper conding for all thread inputs. */
		rd_thread_poll(100);
	}

	rd_thread_cleanup();
}


static void *rd_thread_start_routine (void *arg) {
	rd_thread_t *rdt = arg;
	void *ret;

	rd_currthread = rdt;

	ret = rdt->rdt_start(rdt->rdt_start_arg);

	rd_thread_cleanup();

	return ret;
}


rd_thread_t *rd_thread_create0 (const char *name, pthread_t *pthread) {
	rd_thread_t *rdt;

	rdt = calloc(1, sizeof(*rdt));
  
	if (name)
		rdt->rdt_name = strdup(name);

	rdt->rdt_state = RD_THREAD_S_RUNNING;

	if (pthread)
		rdt->rdt_thread = *pthread;

	return rdt;
}


rd_thread_t *rd_thread_create (const char *name,
			       const pthread_attr_t *attr,
			       void *(*start_routine)(void*),
			       void *arg) {
	rd_thread_t *rdt;

	rdt = rd_thread_create0(name, NULL);

	rdt->rdt_start = start_routine;
	rdt->rdt_start_arg = arg;


	if (pthread_create(&rdt->rdt_thread, attr,
			   rd_thread_start_routine, rdt)) {
		int errno_save = errno;
		if (rdt->rdt_name)
			free(rdt->rdt_name);
		free(rdt);
		errno = errno_save;
		return NULL;
	}

#ifdef PR_SET_NAME
	prctl(PR_SET_NAME, (char *)rdt->rdt_name, 0, 0, 0);
#endif

	return rdt;
}


int rd_threads_create (const char *nameprefix, int threadcount,
		       const pthread_attr_t *attr,
		       void *(*start_routine)(void*),
		       void *arg) {
	int i;
	char *name = alloca(strlen(nameprefix) + 4);
	int failed = 0;

	if (threadcount >= 1000) {
		errno = E2BIG;
		return -1;
	}
		
	for (i = 0 ; i < threadcount ; i++) {
		sprintf(name, "%s%i", nameprefix, i);
		if (!rd_thread_create(name, attr, start_routine, arg))
			failed++;
	}

	if (failed == threadcount)
		return -1;

	return threadcount - failed;
}
