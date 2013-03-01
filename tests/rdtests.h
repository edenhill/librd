#pragma once
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

/**
 * Common helpers for test programs.
 */



/* Globals */
static int tests_dbg RD_UNUSED = 0;


#define TEST_VARS       int fails = 0
#define TEST_RETURN     return fails
#define TEST_EXIT do {							\
		if (fails > 0)						\
			fprintf(stderr, "\n\n%s: %i TESTS FAILED\n",	\
				__FILE__, fails);			\
		exit(fails ? 1 : 0);					\
	} while (0)


#define TEST_ACCUM(call) fails += call

#define TEST_IS_DBG  tests_dbg

#define TEST_DBG(desc...) do {			\
		if (tests_dbg) {					\
			fprintf(stderr, "%s: TEST DEBUG: %s:%i: ",	\
				__FILE__, __FUNCTION__, __LINE__);	\
			fprintf(stderr, desc);				\
			fprintf(stderr, "\n");				\
		}							\
	} while (0)



#define TEST_FAIL(reason...) do {				     \
    fails++;							     \
    fprintf(stderr, "%s: TEST FAILED: %s:%i: ",			     \
	    __FILE__,  __FUNCTION__, __LINE__);			     \
    fprintf(stderr, reason);					     \
    fprintf(stderr, "\n");					     \
  } while (0)

#define TEST_FAIL_RETURN(reason...) do {			     \
    fails++;							     \
    fprintf(stderr, "%s: TEST FAILED: %s:%i: ",			     \
	    __FILE__, __FUNCTION__, __LINE__);			     \
    fprintf(stderr, reason);					     \
    fprintf(stderr, "\n");					     \
    return fails;						     \
  } while (0)


#define TEST_ASSERT(expr) do {						\
	if (!(expr))							\
		TEST_FAIL("test expression \"%s\" failed", #expr);	\
	else								\
		TEST_OK("test expression \"%s\" is true", #expr);	\
	} while (0)

#define TEST_STR_EQ(a, b) do {						\
	if (strcmp(a, b))						\
		TEST_FAIL("test equality comparison of '%s' and '%s' failed", \
			  a, b);					\
	else								\
		TEST_OK("test equality comparison of '%s' and '%s' is true", \
			a, b);						\
	} while (0)
  

#define TEST_STR_NEQ(a, b) do {						\
	if (strcmp(a, b))						\
		TEST_FAIL("test inequality comparison of '%s' "		\
			  "and '%s' failed", a, b);			\
	else								\
		TEST_OK("test inequality comparison of '%s' "		\
			" and '%s' is true", a, b);			\
	} while (0)

#define TEST_INT_EQ(a, b) do {						\
	if ((a) != (b))							\
		TEST_FAIL("test equality comparison of %i and %i failed", \
			  a, b);					\
	else								\
		TEST_OK("test equality comparison of %i and %i is true", \
			a, b);						\
  } while (0)

#define TEST_INT_NEQ(a, b) do {						\
  if ((a) == (b))							\
    TEST_FAIL("test inequality comparison of %i and %i failed", a, b);	\
  else									\
    TEST_OK("test inequality comparison of %i and %i is true", a, b);	\
  } while (0)
  

#define TEST_OK(desc...) do {			\
    if (tests_dbg) {				\
      fprintf(stderr, "%s: TEST OK: %s:%i: ",	\
      __FILE__, __FUNCTION__, __LINE__);	\
      fprintf(stderr, desc);			\
      fprintf(stderr, "\n");			\
    }						\
  } while (0)


#define TEST_INIT do {				\
    if (getenv("LIBRD_TEST_DBG"))		\
      tests_dbg = 1;				\
} while (0)
