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
#include "rdtimer.h"
#include "rdlog.h"
#include "rdstring.h"
#include "rdalert.h"

rd_mutex_t rd_timers_lock = RD_MUTEX_INITIALIZER;
static rd_cond_t  rd_timers_cond;

static LIST_HEAD(, rd_timer_s) rd_timers;
static struct timespec rd_timers_next_ts;


/**
 * NOTE: rd_timers_lock must be held.
 */
static inline void rd_timers_next_update (const rd_timer_t *rt_first) {

	if (rt_first)
		TS_TO_TIMESPEC(&rd_timers_next_ts, rt_first->rt_next);
	else /* No timer, sleep for an hour. */
		TS_TO_TIMESPEC(&rd_timers_next_ts,
			       rd_clock() + (3600LLU * 1000000LLU));

	rd_cond_signal(&rd_timers_cond);
}


/**
 * See rd_timer_destroy() for proper usage.
 * NOTE: rd_timers_lock must be held.
 */
void rd_timer_destroy0 (rd_timer_t *rt) {

	rd_timer_stop0(rt);

	if (rt->rt_called) {
		rt->rt_flags |= RD_TIMER_F_REMOVED;
		return;
	}

	free(rt);
}

/**
 * Thread 'rdt' may be NULL to indicate the current thread.
 */
rd_timer_t *rd_timer_new (rd_timer_type_t type, rd_thread_t *rdt,
			  rd_thread_event_f(*callback), void *ptr) {
	rd_timer_t *rt;

	rt = calloc(1, sizeof(*rt));

	rd_timer_init(rt, type, rdt, callback, ptr);

	return rt;
}


/**
 * Adds an unmanageble timer.
 * Once it has been added it will either be removed automatically
 * after the first period has elapsed (RD_TIMER_ONCE) or kept running
 * forever (RD_TIMER_RECURR).
 */
void rd_timer_add (rd_timer_type_t type, unsigned int interval_ms,
		   rd_thread_t *rdt, rd_thread_event_f(*callback), void *ptr) {
	rd_timer_t *rt;

	rt = rd_timer_new(type, rdt, callback, ptr);
	rt->rt_flags |= RD_TIMER_F_ATOMIC;
	rd_timer_start(rt, interval_ms);
}

static inline int rd_timer_cmp (const rd_timer_t *a, const rd_timer_t *b) {
	return (a->rt_next - b->rt_next);
}


/**
 * NOTE: rd_timers_lock must be held.
 */
void rd_timer_start0 (rd_timer_t *rt, unsigned int interval_ms,
		      int next_update) {

	rd_timer_stop0(rt);

	rt->rt_next = rd_clock() + (interval_ms * 1000);

	if (rt->rt_next < TIMESPEC_TO_TS(&rd_timers_next_ts)) {
		/* This timer will be the next one to fire. */
		LIST_INSERT_HEAD(&rd_timers, rt, rt_link);
		if (next_update)
			rd_timers_next_update(rt);
	} else {
		/* FIXME: Smarter sorted list insertion, i.e. ,at least
		 *        choose whether to start scanning from top or bottom
		 *        of the list */
		LIST_INSERT_SORTED(&rd_timers, rt, rt_link, rd_timer_cmp);
	}
}




/**
 * NOTE: rd_timers_lock must be held.
 */
void rd_timer_stop0 (rd_timer_t *rt) {

	if (rt->rt_next != 0) {
		if (!rt->rt_called &&
		    LIST_FIRST(&rd_timers) == rt)
			rd_timers_next_update(LIST_NEXT(rt, rt_link));
			
		LIST_REMOVE(rt, rt_link);
		rt->rt_next = 0;
	}
}


static rd_thread_event_f(rd_timer_call) {
	rd_timer_t *rt = ptr;

	assert(rt->rt_called > 0);

	rt->rt_rte.rte_callback(rt->rt_rte.rte_ptr);

	/* Post-callback operations */

	rd_mutex_lock(&rd_timers_lock);

	(void)rd_atomic_sub(&rt->rt_called, 1);

	if (rt->rt_flags & RD_TIMER_F_REMOVED && rt->rt_called == 0) {
		/* If the timer was removed while the callback
		 * was being called it was simply put in ..REMOVED
		 * state. It is now time to really remove it. */
		rd_timer_destroy0(rt);

	} else if (rt->rt_type == RD_TIMER_ONCE &&
		   rt->rt_flags & RD_TIMER_F_ATOMIC) {
		/* Destroy atomic one-shot timers */
		rd_timer_destroy0(rt);
	}
		
	rd_mutex_unlock(&rd_timers_lock);
}




static void *rd_timers_run (void *arg) {

	rd_mutex_lock(&rd_timers_lock);

	while (1) {
		rd_ts_t now;
		rd_timer_t *rt;
		struct timespec timeout;

		TAILQ_HEAD(, rd_timer_s) callouts =
			TAILQ_HEAD_INITIALIZER(callouts);
		
		now = rd_clock();

		while ((rt = LIST_FIRST(&rd_timers)) &&
		       rt->rt_next <= now) {

			LIST_REMOVE(rt, rt_link);
			rt->rt_next = 0;

			TAILQ_INSERT_TAIL(&callouts, rt, rt_callout_link);

			if (rt->rt_called > 10 &&
			    !(rt->rt_called % 10)) {
				/* More than X events and 15s are already
				 * enqueued for this timer. The thread
				 * is probably stalled. */
				rd_alert(RD_ALERT_THREAD_STALL, LOG_WARNING,
					 rd_tsprintf("Timer %p has %i events "
						     "enqueued on thread \"%s\""
						     "(%p): thread stalled?",
						     rt, rt->rt_called,
						     rt->rt_thread->rdt_name,
						     rt->rt_thread),
					 rt->rt_thread,
					 (rt->rt_called * rt->rt_interval) /
					 1000);
			}

			/* Indicate with called counter that we're in
			 * the callback (and without locks). This prohibits
			 * some other thread from freeing the timer
			 * while we're calling the callback. */
			(void)rd_atomic_add(&rt->rt_called, 1);
			rd_thread_event_add(rt->rt_thread, rd_timer_call, rt);
		}

		/* Restart recurring timers */
		TAILQ_FOREACH(rt, &callouts, rt_callout_link) {
			if (rt->rt_type == RD_TIMER_RECURR) {
				/* Restart recurring timers. */
				rd_timer_start0(rt, rt->rt_interval, 0);
			}
		}

		rd_timers_next_update(LIST_FIRST(&rd_timers));

		timeout = rd_timers_next_ts;
		rd_cond_timedwait(&rd_timers_cond, &rd_timers_lock, &timeout);
	}

	return NULL;
}

void rd_timers_init (void) {

	pthread_condattr_t attr;
	pthread_condattr_init(&attr);
#ifndef __APPLE__
	pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
#endif
	rd_cond_init(&rd_timers_cond, &attr);

	rd_thread_create(NULL, "rd:timers", NULL, rd_timers_run, NULL);
}
