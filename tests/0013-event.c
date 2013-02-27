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
#include "rdevent.h"
#include "rdthread.h"

#include "rdtests.h"


static int calltrace = 0;
static int callfails = 0;

static void func0 (void) {
	calltrace |= (1 << 0);
}

static void func1 (void *arg1) {
	TEST_VARS;

	if (arg1 != (void *)0x11111111)
		TEST_FAIL("func1 arg1 is wrong %p", arg1);

	calltrace |= (1 << 1);

	callfails += fails;
}

static void func2 (void *arg1, void *arg2) {
	TEST_VARS;

	if (arg1 != (void *)0x11111111)
		TEST_FAIL("func2 arg1 is wrong %p", arg1);
	if (arg2 != (void *)0x22222222)
		TEST_FAIL("func2 arg2 is wrong %p", arg2);

	calltrace |= (1 << 2);

	callfails += fails;
}

static void func3 (void *arg1, void *arg2, void *arg3) {
	TEST_VARS;

	if (arg1 != (void *)0x11111111)
		TEST_FAIL("func3 arg1 is wrong %p", arg1);
	if (arg2 != (void *)0x22222222)
		TEST_FAIL("func3 arg2 is wrong %p", arg2);
	if (arg3 != (void *)0x33333333)
		TEST_FAIL("func3 arg3 is wrong %p", arg3);

	calltrace |= (1 << 3);

	callfails += fails;
}

static void func4 (void *arg1, void *arg2, void *arg3, void *arg4) {
	TEST_VARS;

	if (arg1 != (void *)0x11111111)
		TEST_FAIL("func4 arg1 is wrong %p", arg1);
	if (arg2 != (void *)0x22222222)
		TEST_FAIL("func4 arg2 is wrong %p", arg2);
	if (arg3 != (void *)0x33333333)
		TEST_FAIL("func4 arg3 is wrong %p", arg3);
	if (arg4 != (void *)0x44444444)
		TEST_FAIL("func4 arg4 is wrong %p", arg4);

	calltrace |= (1 << 4);

	callfails += fails;
}


static int test_event_func_call (void) {
	TEST_VARS;

	rd_thread_func_call0(rd_mainthread, func0);
	rd_thread_func_call1(rd_mainthread, func1, (void *)0x11111111);
	rd_thread_func_call2(rd_mainthread, func2,
			     (void *)0x11111111, (void *)0x22222222);
	rd_thread_func_call3(rd_mainthread, func3,
			     (void *)0x11111111, (void *)0x22222222,
			     (void *)0x33333333);
	rd_thread_func_call4(rd_mainthread, func4,
			     (void *)0x11111111, (void *)0x22222222,
			     (void *)0x33333333, (void *)0x44444444);

	while (calltrace != 0b11111)
		rd_thread_poll(100);

	fails += callfails;

	TEST_RETURN;
}


static void test_timeout (int sig) {
	TEST_VARS;
	TEST_FAIL("Test timed out");
	exit(1);
}

int main (int argc, char **argv) {
	TEST_VARS;

	TEST_INIT;

	signal(SIGALRM, test_timeout);
	alarm(3);

	rd_init();

	fails += test_event_func_call();

	TEST_EXIT;
}

