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

#include "rdavl.h"


struct elm {
	const char *e_name;
	rd_avl_node_t e_link;
	int e_expect;
};

static int elm_cmp (const void *_a, const void *_b) {
	const struct elm *a = _a, *b = _b;
	return strcmp(a->e_name, b->e_name);
}

static void foreach_check_count(void *velm, void *opaque) {
	struct elm *elm = velm;
	int *count = opaque;
	size_t i;
	const char *expected[] = {
		"one",
		"two",
		"three",
		"four",
		"five",
	};

	for (i = 0; i < RD_ARRAYSIZE(expected); ++i) {
		if (0 == strcmp(expected[i], elm->e_name)) {
			(*count)++;
			return;
		}
	}
}

static int count_check(rd_avl_t *ravl, int expected_count) {
	int count = 0;

	/* Count elements with foreach */
	RD_AVL_FOREACH(&ravl, foreach_check_count, &count);
	if (count != expected_count) {
		return 1;
	}

	return 0;
}

static int test_avl (void) {
	int fails = 0;
	struct elm elms[] = {
		{ "one" },
		{ "two" },
		{ "three" },
		{ "four" },
		{ "five" },
		{ NULL }
	};
	struct elm *e, *e2;
	struct elm skel;
	rd_avl_t ravl;

	rd_avl_init(&ravl, elm_cmp, RD_AVL_F_LOCKS);

	for (e = elms ; e->e_name ; e++) {
		RD_AVL_INSERT(&ravl, e, e_link);
		e->e_expect = 1;
	}

	/* Scan to see that they are now found. */
	fails += count_check(&ravl, 5);
	for (e = elms ; e->e_name ; e++) {
		struct elm *e2;
		struct elm skel2 = { .e_name = e->e_name };

		if (!(e2 = RD_AVL_FIND(&ravl, e))) {
			printf("%s:%i: RD_AVL_FIND(..\"%s\") returned NULL\n",
			       __FUNCTION__,__LINE__, e->e_name);
			fails++;
		} else if (e2 != e) {
			printf("%s:%i: RD_AVL_FIND(..\"%s\") returned \"%s\"\n",
			       __FUNCTION__,__LINE__, e->e_name, e2->e_name);
			fails++;
		}

		/* Reproduce with skeleton to make sure its the value
		 * that is compared and not the pointers. */
		if (!(e2 = RD_AVL_FIND(&ravl, &skel2))) {
			printf("%s:%i: RD_AVL_FIND(skel..\"%s\") "
			       "returned NULL\n",
			       __FUNCTION__,__LINE__, e->e_name);
			fails++;
		} else if (e2 != e) {
			printf("%s:%i: RD_AVL_FIND(skel..\"%s\") "
			       "returned \"%s\"\n",
			       __FUNCTION__,__LINE__, e->e_name, e2->e_name);
			fails++;
		}
	}

	/* Remove some stuff */
	skel.e_name = "three";
	RD_AVL_REMOVE_ELM(&ravl, &skel);
	elms[2].e_expect = 0;

	skel.e_name = "three";
	RD_AVL_REMOVE_ELM(&ravl, &skel);


	/* Verify that they're now gone. */
	fails += count_check(&ravl, 4);
	for (e = elms ; e->e_name ; e++) {
		if (!(e2 = RD_AVL_FIND(&ravl, e))) {
			if (e->e_expect) {
				printf("%s:%i: RD_AVL_FIND(..\"%s\") "
				       "returned NULL\n",
				       __FUNCTION__,__LINE__, e->e_name);
				fails++;
			}
		} else if (e2 != e) {
			printf("%s:%i: RD_AVL_FIND(..\"%s\") returned \"%s\"\n",
			       __FUNCTION__,__LINE__, e->e_name, e2->e_name);
			fails++;
		}

		if (e2 && !e->e_expect) {
			printf("%s:%i: RD_AVL_FIND(..\"%s\") "
			       "returned \"%s\" but expected NULL\n",
			       __FUNCTION__,__LINE__, e->e_name,
			       e2->e_name);
			fails++;
		}
	}

	/* Insert duplicate. */
	skel.e_name = "two";
	skel.e_expect = 1;
	e2 = RD_AVL_INSERT(&ravl, &skel, e_link);
	elms[1].e_expect = 0;
	if (e2 != &elms[1]) {
		printf("%s:%i: RD_AVL_INSERT(dupl..\"%s\") "
		       "returned %p, should've returned existing entry %p\n",
		       __FUNCTION__,__LINE__,
		       skel.e_name, e2, &elms[1]);
		fails++;
	}

	/* Scan to see its correct. */
	fails += count_check(&ravl, 4);
	for (e = elms ; e->e_name ; e++) {

		if (!(e2 = RD_AVL_FIND(&ravl, e))) {
			if (e->e_expect) {
				printf("%s:%i: RD_AVL_FIND(..\"%s\") "
				       "returned NULL\n",
				       __FUNCTION__,__LINE__, e->e_name);
				fails++;
			}
		} else if (strcmp(e2->e_name, e->e_name)) {
			printf("%s:%i: RD_AVL_FIND(..\"%s\") returned \"%s\"\n",
			       __FUNCTION__,__LINE__, e->e_name, e2->e_name);
			fails++;
		}

		if (e2 && !e2->e_expect) {
			printf("%s:%i: RD_AVL_FIND(..\"%s\") "
			       "returned \"%s\" but expected NULL\n",
			       __FUNCTION__,__LINE__, e->e_name,
			       e2->e_name);
			fails++;
		}
	}

	rd_avl_destroy(&ravl);

	return fails;
}

int main (int argc, char **argv) {
	int fails = 0;

	fails += test_avl();

	return fails ? 1 : 0;
}
