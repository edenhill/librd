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
#include "rdopt.h"

struct storeval {
	char *c;
	int   d;
	int   elf;
	char *path;
};

static int opt_tests (void) {
	int fails = 0;
	struct storeval store;
	rd_opt_t myopts[] = {
		{ RD_OPT_NIL|RD_OPT_REQ, 'r', "required", 0 },
		{ 0, 'a', "alpha" },
		{ RD_OPT_NIL, 0, "beta" },
		{ RD_OPT_STR, 'c', "csar", 1, &store.c },
		{ RD_OPT_BOOL, 'd', NULL, 0, &store.d },
		{ RD_OPT_INT, 0, "--elf", 1, &store.elf },
		{ RD_OPT_PATH, 'p', "path", 1,  &store.path },
		{ RD_OPT_END },
	};
	struct {
		enum {
			_PASS = 0,
			_FAIL,
		} expect;
		int argc;
		const char *argv[32];
		int check_store;
		struct storeval store;
	} t[] = {
		{ _FAIL, 3, { "", "--mach", "2" } },
		{ _PASS, 7, { "", "-c", "2", "-d", "--path", "/tmp",
			      "--required" },
		  1, { c: "2", d: 1, path: "/tmp" } },
		{ _FAIL, 4, { "", "--required",
			      "--path", "/probably wont exist"},
		  1, { path: NULL } },
	};
	int i;

	for (i = 0 ; i < RD_ARRAYSIZE(t) ; i++) {
		const char *ret;
		int argi = -1;

		memset(&store, 0, sizeof(store));

		ret = rd_opt_parse(myopts, t[i].argc, t[i].argv, &argi);

		if (t[i].expect == _PASS && ret) {
			printf("rd_opt test #%i failed: expected PASS, got "
			       "error: %s\n", i, ret);
			fails++;
		} else if (t[i].expect == _FAIL && !ret) {
			printf("rd_opt test #%i failed: expected FAIL, got "
			       "success\n", i);
			fails++;
		} else if (t[i].check_store) {
			if (t[i].store.c && (!store.c ||
					     strcmp(t[i].store.c, store.c))) {
				printf("rd opt test #%i failed: "
				       "opt value c should be '%s' but is "
				       "'%s'\n",
				       i, t[i].store.c, store.c);
				fails++;
			}

			if (t[i].store.path && (!store.path ||
					     strcmp(t[i].store.path,
						    store.path))) {
				printf("rd opt test #%i failed: "
				       "opt value path should be '%s' but is "
				       "'%s'\n",
				       i, t[i].store.path, store.path);
				fails++;
			}

			if (t[i].store.d != store.d) {
				printf("rd opt test #%i failed: "
				       "opt value d should be %i but is %i\n",
				       i, t[i].store.d, store.d);
				fails++;
			}

			if (t[i].store.elf != store.elf) {
				printf("rd opt test #%i failed: "
				       "opt value elf should be %i but "
				       "is %i\n",
				       i, t[i].store.elf, store.elf);
				fails++;
			}
		}

	}
		

	return fails;
}


int main (int argc, char **argv) {
	int fails = 0;

	fails += opt_tests();

	return fails ? 1 : 0;
}
