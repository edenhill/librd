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
#include "rdqueue.h"
#include "rdevent.h"
#include "rdtimer.h"

/**
 * This test has a set of caller threads and worker threads.
 * The caller threads allocate an object and adds it to the fifoq,
 * the fifoq is served by the worker threads that perform some work
 * and then pass the object through thread events (extra-thread callbacks)
 * to the main thread, which count the number of objects and exits.
 * To catch the case where too many objects are passed the main thread
 * waits a short while after receiving the last expected object before
 * exiting, as to catch any late duplicates.
 */

const int numcallers = 4;
const int numobjs = 1000; /* per caller */
int numobjsdone = 0;
int objid = 0;

struct myobj {
	int id;
	int a,b,c;
};

static rd_fifoq_t fifoq;

static rd_thread_event_f(really_done) {
	rd_thread_exit();
}

static rd_thread_event_f(obj_done) {
	struct myobj *obj = ptr;

	assert(pthread_equal(pthread_self(), rd_mainthread->rdt_thread));

	assert(obj->id <= numcallers * numobjs);

	free(obj);

	if (++numobjsdone >= numcallers * numobjs) {
		rd_timer_add(RD_TIMER_ONCE, 100/*ms*/,
			     rd_mainthread, really_done, NULL);
	}
}


static void *worker (void *ignore) {

	while (1) {
		rd_fifoq_elm_t *rfqe;
		struct myobj *obj;

		rfqe = rd_fifoq_pop_wait(&fifoq);

		obj = rfqe->rfqe_ptr;
		obj->c = obj->a + obj->b;

		rd_thread_event_add(rd_mainthread, obj_done, obj);

		rd_fifoq_elm_release(&fifoq, rfqe);

	}

	return NULL;
}


static void *caller (void *ignore) {
	int i;

	for (i = 0 ; i < numobjs ; i++) {
		struct myobj *obj = calloc(1, sizeof(*obj));

		obj->id = rd_atomic_add(&objid, 1);
		obj->a = 1;
		obj->b = i;

		rd_fifoq_add(&fifoq, obj);
	}

	sleep(5);
	return NULL;
}


static rd_thread_event_f(test_timeout) {
	printf("Test timed out\n");
	exit(1);
}
	

int main (int argc, char **argv) {

	rd_init();

	rd_timer_add(RD_TIMER_ONCE, 2000, NULL, test_timeout, NULL);

	rd_fifoq_init(&fifoq);

	rd_threads_create("worker", 4, NULL, worker, NULL);
	rd_threads_create("caller", numcallers, NULL, caller, NULL);

	rd_thread_dispatch();

	if (numobjsdone != numcallers * numobjs) {
		printf("%i objects are done, but %i expected!\n",
		       numobjsdone, numcallers * numobjs);
		return 1;
	}

	return 0;
}
