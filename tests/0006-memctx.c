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
#include "rdmem.h"
#include "rdunits.h"

static int test_memctxs (void) {
	int fails = 0;
	rd_memctx_t rmc;
	const int num = 200;
	void *ptr[num];
	int i;
	int sum = 0;
	rd_memctx_stats_t stats;

	rd_memctx_init(&rmc, "test1", 0);
	
	/* Test allocations */
	for (i = 0 ; i < num ; i++) {
		int sz = 16 + (i % 100) * 100;
		if (i & 1)
			ptr[i] = rd_memctx_malloc(&rmc, sz);
		else
			ptr[i] = rd_memctx_calloc(&rmc, 1, sz);
		sum += sz;
	}

	/* Verify counters */
	rd_memctx_stats(&rmc, &stats);

	if (stats.out != num) {
		printf("%s:%i: failed: stats.out %i != real.out %i\n",
		       __FUNCTION__,__LINE__,
		       stats.out, num);
		fails++;
	}

	if (stats.bytes_out != sum) {
		printf("%s:%i: failed: stats.bytes_out %zd != real.sum %i\n",
		       __FUNCTION__,__LINE__,
		       stats.bytes_out, sum);
		fails++;
	}

	/* Test sized frees */
	for (i = 0 ; i < num ; i++) {
		int sz = 16 + (i % 100) * 100;
		sum -= sz;
		rd_memctx_freesz(&rmc, ptr[i], sz);
	}

	/* Verify counters */
	rd_memctx_stats(&rmc, &stats);
	if (stats.out != 0) {
		printf("%s:%i: failed: stats.out %i != 0\n",
		       __FUNCTION__,__LINE__,
		       stats.out);
		fails++;
	}

	if (stats.bytes_out != 0) {
		printf("%s:%i: failed: stats.bytes_out %zd != 0\n",
		       __FUNCTION__,__LINE__,
		       stats.bytes_out);
		fails++;
	}
	
	rd_memctx_destroy(&rmc);


	/*
	 * Test tracked allocations
	 */
	 
	rd_memctx_init(&rmc, "test2:track", RD_MEMCTX_F_TRACK);

	/* Test allocations */
	for (i = 0 ; i < num ; i++) {
		int sz = 16 + (i % 100) * 100;
		if (i & 1)
			ptr[i] = rd_memctx_malloc(&rmc, sz);
		else
			ptr[i] = rd_memctx_calloc(&rmc, 1, sz);
		sum += sz;
	}

	/* Verify counters */
	rd_memctx_stats(&rmc, &stats);

	if (stats.out != num) {
		printf("%s:%i: failed: stats.out %i != real.out %i\n",
		       __FUNCTION__,__LINE__,
		       stats.out, num);
		fails++;
	}

	if (stats.bytes_out != sum) {
		printf("%s:%i: failed: stats.bytes_out %zd != real.sum %i\n",
		       __FUNCTION__,__LINE__,
		       stats.bytes_out, sum);
		fails++;
	}

	/* Test cleanup */
	rd_memctx_freeall(&rmc);

	/* Verify counters */
	rd_memctx_stats(&rmc, &stats);
	if (stats.out != 0) {
		printf("%s:%i: failed: stats.out %i != 0\n",
		       __FUNCTION__,__LINE__,
		       stats.out);
		fails++;
	}

	if (stats.bytes_out != 0) {
		printf("%s:%i: failed: stats.bytes_out %zd != 0\n",
		       __FUNCTION__,__LINE__,
		       stats.bytes_out);
		fails++;
	}
	
	rd_memctx_destroy(&rmc);

	/*
	 * Test free tracked pointers
	 */
	rd_memctx_init(&rmc, "test3:free_track", RD_MEMCTX_F_TRACK);

	/* Test allocations */
	sum = 0;
	for (i = 0 ; i < num ; i++) {
		int sz = 16 + (i % 100) * 100;
		if (i & 1)
			ptr[i] = rd_memctx_malloc(&rmc, sz);
		else
			ptr[i] = rd_memctx_calloc(&rmc, 1, sz);
		sum += sz;
	}

	/* Verify counters */
	rd_memctx_stats(&rmc, &stats);
	if (stats.out != num) {
		printf("%s:%i: failed: stats.out %i != real.out %i\n",
		       __FUNCTION__,__LINE__,
		       stats.out, num);
		fails++;
	}

	if (stats.bytes_out != sum) {
		printf("%s:%i: failed: stats.bytes_out %zd != real.sum %i\n",
		       __FUNCTION__,__LINE__,
		       stats.bytes_out, sum);
		fails++;
	}

	/* Test few cleanup */
	for (i = 0 ; i < num ; i+=20) {
		int sz = 16 + (i % 100) * 100;
		sum -= sz;
		rd_memctx_freesz(&rmc, ptr[i], sz);
	}

	/* Verify counters */
	rd_memctx_stats(&rmc, &stats);
	if (stats.out != 190) {
		printf("%s:%i: failed: stats.out %i != 190\n",
		       __FUNCTION__,__LINE__,
		       stats.out);
		fails++;
	}
	
	/* Total cleanup */
	rd_memctx_freeall(&rmc);

	/* Verify counters */
	rd_memctx_stats(&rmc, &stats);
	if (stats.out != 0) {
		printf("%s:%i: failed: stats.out %i != 0\n",
		       __FUNCTION__,__LINE__,
		       stats.out);
		fails++;
	}

	if (stats.bytes_out != 0) {
		printf("%s:%i: failed: stats.bytes_out %zd != 0\n",
		       __FUNCTION__,__LINE__,
		       stats.bytes_out);
		fails++;
	}
	
	rd_memctx_destroy(&rmc);


	return fails;
}


static int test_alloc_struct (void) {
	struct test {
		int a;
		char *b;
		char *c;
		int d[16];
		int *e;
		int f;
	} *test;
	const char *bs = "the b field";
	const char cs[] = "abcdefghijklmnopqrstuvwxyzåäö0123456789!?";
	const int es[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
	int fails = 0;

	rd_calloc_struct(&test, sizeof(*test),
			 -1, "the b field", &test->b,
			 sizeof(cs), cs, &test->c,
			 sizeof(es), es, &test->e,
			 RD_MEM_END_TOKEN);

	if (!test) {
		printf("%s:%i failed #1: no pointer returned\n",
		       __FUNCTION__,__LINE__);
		return 1;
	}

	if (test->b == NULL ||
	    strcmp(test->b, bs)) {
		printf("%s:%i: failed #2: test->b is %s, should be %s;\n",
		       __FUNCTION__,__LINE__, test->b, bs);
		fails++;
	}


	if (test->c == NULL ||
	    strcmp(test->c, cs)) {
		printf("%s:%i: failed #3: test->c is %s, should be %s;\n",
		       __FUNCTION__,__LINE__, test->c, cs);
		fails++;
	}
	

	if (test->e == NULL ||
	    memcmp(test->e, es, sizeof(es))) {
		printf("%s:%i: failed #4: test->e (%p) doesnt match es;\n",
		       __FUNCTION__,__LINE__, test->e);
		fails++;
	}

	free(test);

	return 0;
}
int main (int argc, char **argv) {
	int fails = 0;

	fails += test_memctxs();

	fails += test_alloc_struct();
	return fails ? 1 : 0;
}
