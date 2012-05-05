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
#include "rdunits.h"


static int size2str_tests (void) {
	int fails = 0;
	static const struct {
		uint64_t    val;
		int         si;
		const char *exp;
	} in[] = {
		{ 1023, 0, "1023B" },
		{ 1024, 0, "1KiB" },
		{ 1025, 0, "1KiB" },
		{ 149982691759LLU, 0, "139.68GiB" },
		{ 213921076233013234LLU, 0, "190PiB" },
		/* SI units */
		{ 1023, 1, "1.02KB" },
		{ 2048, 1, "2.05KB" },
		{ 1234567LLU, 1, "1.23MB" },
		{ 189890123456794833LLU, 1, "189.89PB" },
		{ 0, 0, NULL },
	};
	int i = 0;
	const char *t;
	
	
	do {
		t = rd_size2str(in[i].val, in[i].si, "B");
		if (strcmp(t, in[i].exp)) {
			printf("rd_size2str() test #%i failed: "
			       "val %" PRIu64 ", si=%i expected '%s', "
			       "but got '%s'\n",
			       i, in[i].val, in[i].si, in[i].exp, t);
			fails++;
		}

	} while (in[++i].exp);

	return fails;
}


int main (int argc, char **argv) {
	int fails = 0;

	rd_init();

	fails += size2str_tests();

	return fails ? 1 : 0;
}
