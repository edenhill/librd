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



#define rd_thread_event_f(F)  void (F) (void *ptr)

typedef struct rd_thread_event_s {
	rd_thread_event_f(*rte_callback);
	void        *rte_ptr;
} rd_thread_event_t;



/**
 * Enqueue event (callback call) on thread 'rdt'.
 * Requires 'rdt' to call rd_thread_dispatch().
 */
static void rd_thread_event_add (rd_thread_t *rdt,
				 rd_thread_event_f(*callback),
				 void *ptr) RD_UNUSED;

static void rd_thread_event_add (rd_thread_t *rdt,
				 rd_thread_event_f(*callback),
				 void *ptr) {
	rd_thread_event_t *rte = malloc(sizeof(*rte));

	rte->rte_callback = callback;
	rte->rte_ptr = ptr;

	rd_fifoq_add(&rdt->rdt_eventq, rte);
}


/**
 * Calls the callback and destroys the event.
 */
static void rd_thread_event_call (rd_thread_event_t *rte) RD_UNUSED;
static void rd_thread_event_call (rd_thread_event_t *rte) {

	rte->rte_callback(rte->rte_ptr);

	free(rte);
}
