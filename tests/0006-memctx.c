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


	return fails;
}


int main (int argc, char **argv) {
	int fails = 0;

	fails += test_memctxs();

	return fails ? 1 : 0;
}
