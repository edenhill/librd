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

#pragma once

#include "rdthread.h"
#include "rdevent.h"

typedef enum {
	RD_TIMER_ONCE,
	RD_TIMER_RECURR,
} rd_timer_type_t;

typedef struct rd_timer_s {
	LIST_ENTRY(rd_timer_s)  rt_link;         /* Waiting-to-expire list */
	TAILQ_ENTRY(rd_timer_s) rt_callout_link; /* Temporary call list */
	rd_thread_t       *rt_thread;   /* Thread to call callback in */
	rd_thread_event_t  rt_rte;      /* Callback skeleton */
	rd_timer_type_t    rt_type;     /* Timer type */
	unsigned int       rt_interval; /* Timer interval in milliseconds */
	rd_ts_t            rt_next;     /* Calculated next firing time in
					 * absolute timestamp (usec) */
	int                rt_called;   /* Timer has fired and callback is
					 * scheduled or executing.
					 * In this state the timer will not
					 * be freed on rd_timer_destroy(), just
					 * flagged for future removal.
					 * This value may be higher than one
					 * if the timer interval is faster
					 * than the fire events can be
					 * handled. */

	int                rt_flags;
#define RD_TIMER_F_ATOMIC  0x1  /* Allocated by librd, will be freed
				 * automatically after callback has fired.
				 * Application must not keep a reference
				 * to the timer since it allows no such
				 * locking. */
#define RD_TIMER_F_REMOVED 0x4  /* Timer was removed while being handled.
				 * Real destruction is delayed. */
} rd_timer_t;


extern rd_mutex_t rd_timers_lock;

void rd_timer_destroy0 (rd_timer_t *rt);
void rd_timer_start0 (rd_timer_t *rt, unsigned int interval_ms);
void rd_timer_stop0 (rd_timer_t *rt);

static void rd_timer_init (rd_timer_t *rt, rd_timer_type_t type,
			   rd_thread_t *rdt, rd_thread_event_f(*callback),
			   void *ptr) RD_UNUSED;
static void rd_timer_init (rd_timer_t *rt, rd_timer_type_t type,
			   rd_thread_t *rdt, rd_thread_event_f(*callback),
			   void *ptr) {
	memset(rt, 0, sizeof(*rt));

	rt->rt_type = type;
	rt->rt_thread = rdt ? : rd_currthread_get();
	rt->rt_rte.rte_callback = callback;
	rt->rt_rte.rte_ptr = ptr;
}

/**
 * Initializes an rd_timer_t.
 * It is pointless to provide an rt_thread argument here since no
 * threads will have been created. 
 * Use RD_TIMER_SET_THREAD() later on instead if an alternate thread is
 * required.
 */
#define RD_TIMER_INITIALIZER(TYPE,CB,PTR)			\
	{ rt_type: TYPE, rt_rte: { rte_callback: CB, rte_ptr: PTR } }

#define RD_TIMER_SET_THREAD(rt,thread) ((rt)->rt_thread = (thread))

rd_timer_t *rd_timer_new (rd_timer_type_t type, rd_thread_t *rdt,
			  rd_thread_event_f(*callback), void *ptr);
void rd_timer_add (rd_timer_type_t type, unsigned int interval_ms,
		   rd_thread_t *rdt, rd_thread_event_f(*callback), void *ptr);


static inline void rd_timer_start (rd_timer_t *rt, unsigned int interval_ms)
	RD_UNUSED;
static inline void rd_timer_start (rd_timer_t *rt, unsigned int interval_ms) {
	rd_mutex_lock(&rd_timers_lock);

	rt->rt_interval = interval_ms;
	rd_timer_start0(rt, interval_ms);

	rd_mutex_unlock(&rd_timers_lock);
}

static inline void rd_timer_stop (rd_timer_t *rt) RD_UNUSED;
static inline void rd_timer_stop (rd_timer_t *rt) {
	rd_mutex_lock(&rd_timers_lock);
	rd_timer_stop0(rt);
	rd_mutex_unlock(&rd_timers_lock);
}


static inline int rd_timer_next (const rd_timer_t *rt) RD_UNUSED;
static inline int rd_timer_next (const rd_timer_t *rt) {
	rd_ts_t now;
	int next;

	rd_mutex_lock(&rd_timers_lock);

	if (!rt->rt_next) {
		/* Timer is not armed */
		next = 0;
	} else if ((now = rd_clock()) < rt->rt_next) {
		/* Timer is delayed, indicate immediate firing */
		next = 1;
	} else {
		next = (int)((now - rt->rt_next) / 1000);
	}

	rd_mutex_unlock(&rd_timers_lock);
	return next;
}

static inline void rd_timer_destroy (rd_timer_t *rt) RD_UNUSED;
static inline void rd_timer_destroy (rd_timer_t *rt) {
	rd_mutex_lock(&rd_timers_lock);
	rd_timer_destroy0(rt);
	rd_mutex_unlock(&rd_timers_lock);
}

