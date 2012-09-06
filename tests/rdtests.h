#pragma once
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

/**
 * Common helpers for test programs.
 */
#define TEST_VARS       int fails = 0
#define TEST_RETURN     return fails
#define TEST_EXIT       exit(fails ? 1 : 0)

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


