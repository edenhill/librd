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
#include "rdbits.h"


struct bitseries {
	uint64_t val;
	enum {
		_SET = 0,
		_FAIL,
		_DONTSET,
		_END,
	} expect;
};

static int bitvec_tests (void) {
	int fails = 0;
	rd_bitvec_t rbv;
	static const struct {
		int max;
		struct bitseries series[32];
	} t[] = {
		{ 31, 
		  { {0}, {1}, {2}, {3}, {4, _DONTSET}, {9},
		    {22}, {22}, {30}, {31},
		    {32, _FAIL},
		    { 0, _END } },
		},
		{ 32,
		  { {31}, {32}, { 0, _END } }
		},
		{ 2000,
		  { {2001, _FAIL}, {23512351235LLU, _FAIL}, {1}, {1920},
		    {1999, _DONTSET}, {2000}, {0},
		    { 0, _END } },
		}
	};
	static const char *expectstr[] = { "SET", "FAIL", "DONTSET", "END" };
	int i, j;
	uint64_t bit;


#define FAIL(fmt...) do {						\
		printf("%s:%i: "					\
		       "bitvec test #%i FAILED: "			\
		       "series[%i] = %llu (expect=%s): ",		\
		       __FUNCTION__,__LINE__,				\
		       i, j, t[i].series[j].val,			\
		       expectstr[t[i].series[j].expect]);		\
		printf(fmt);						\
		printf("\n");						\
		fails++;						\
     } while (0)


	for (i = 0 ; i < RD_ARRAYSIZE(t) ; i++) {
		rd_bitvec_init(&rbv, RD_BITVEC_STATIC, t[i].max);

		/* Set test */
		for (j = 0 ; j < RD_ARRAYSIZE(t[i].series) ; j++) {
			if (t[i].series[j].expect == _END)
				break;

			if (t[i].series[j].expect != _DONTSET)
				rd_bitvec_set(&rbv, t[i].series[j].val);

			if (rd_bitvec_test(&rbv, t[i].series[j].val)) {
				if (t[i].series[j].expect != _SET)
					FAIL("bit should not be set but is");
			} else if (t[i].series[j].expect == _SET)
				FAIL("bit should be set but isnt");
		}

		/* Check test */
		for (j = 0 ; j < RD_ARRAYSIZE(t[i].series) ; j++) {
			if (t[i].series[j].expect == _END)
				break;

			if (rd_bitvec_test(&rbv, t[i].series[j].val)) {
				if (t[i].series[j].expect != _SET)
					FAIL("bit should not be set but is");
			} else if (t[i].series[j].expect == _SET)
				FAIL("bit should be set but isnt");
		}

		/* Reset test */
		for (j = 0 ; j < RD_ARRAYSIZE(t[i].series) ; j++) {
			if (t[i].series[j].expect == _END)
				break;

			if (t[i].series[j].expect != _DONTSET)
				rd_bitvec_reset(&rbv, t[i].series[j].val);

			if (rd_bitvec_test(&rbv, t[i].series[j].val)) {
				if (t[i].series[j].expect == _SET)
					FAIL("bit should not be set but is");
			}
		}

		rd_bitvec_free(&rbv);
	}

#undef FAIL
#define FAIL(fmt...) do {						\
		printf("%s:%i: "					\
		       "bitvec test FAILED: ",				\
		       __FUNCTION__,__LINE__);				\
		printf(fmt);						\
		printf("\n");						\
		fails++;						\
     } while (0)

	/* ffs/fsl */
	rd_bitvec_init(&rbv, RD_BITVEC_STATIC, 300);
	rd_bitvec_set(&rbv, 17);
	rd_bitvec_set(&rbv, 168);
	rd_bitvec_set(&rbv, 288);

	if ((bit = rd_bitvec_ffs(&rbv)) != 18)
		FAIL("ffs returned %llu, should've been 18", bit);
	if ((bit = rd_bitvec_fls(&rbv)) != 289)
		FAIL("fls returned %llu, should've been 289", bit);
	
	rd_bitvec_set(&rbv, 300);

	if ((bit = rd_bitvec_fls(&rbv)) != 301)
		FAIL("fls returned %llu, should've been 301", bit);

	rd_bitvec_free(&rbv);

	return fails;
}


int main (int argc, char **argv) {
	int fails = 0;

	fails += bitvec_tests();

	return fails ? 1 : 0;
}
