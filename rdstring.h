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

#pragma once

#include <string.h>
#include <stdarg.h>
#include "rdmem.h"

#define RD_TSPRINTF_BUFCNT    64  /* Number of simultaneously valid
				   * thread-local buffers returned by 
				   * rd_tsprintf(). */

/**
 * Returns a temporary string (from a circular buffer wrapping
 * at RD_TSPRINTF_BUFCNT thread-local calls) containing the formatted
 * string as described by 'format' and the following var-args.
 *
 * To free the allocated memory, rd_string_thread_cleanup() must be called
 * prior to destroying the thread.
 * This is done automatically by rd_thread_exit() or rd_thread_cleanup().
 */
char *rd_tsprintf (const char *format, ...)
	__attribute__((format (printf, 1, 2)));


/**
 * Wrapper for snprintf() that concatenates the current content of 'str'
 * with the one described in 'format' and then returns the full length of 'str'.
 */
int rd_snprintf_cat (char *str, size_t size, const char *format, ...)
	__attribute__((format (printf, 3, 4)));



/**
 * Looks for any of the characters in '*delimiters' and returns
 * the pointer to it.
 * One big difference from strchr() is that it supports matching '\0'
 * if `match_eol' is set, thus allowing
 * conveniant scan-to-token(s)-or-end-of-buffer operations.
 * The other difference is that supports multiple tokens to look for.
 * 'size' is either the length of 's' or '-1' if it is to scan to '\0'.
 */
char *rd_strnchrs (const char *s, ssize_t size, const char *delimiters,
		   int match_eol);



/**
 * Acts like strspn(3) (if accept=1) or strcspn(3) (if accept=0)
 * but takes a sized input string (rather than a nul-terminated one)
 * and the accept/reject characters are specified in a 1:1 map instead.
 * If 'accept' is 1 it acts like strspn(3), if 0 like strcspn(3).
 */
size_t rd_strnspn_map (const char *s, size_t size,
		       int accept, const char map[256]);

/**
 * Acts like strspn(3) but allows for non-terminated strings.
 */
size_t rd_strnspn (const char *s, size_t size, const char *accept);

/**
 * Acts like strcspn(3) but allows for non-terminated strings.
 */
size_t rd_strncspn (const char *s, size_t size, const char *reject);



/**
 * strncmp() wrapper which takes two input lengths.
 */
static inline int rd_strnncmp (const char *s1, size_t n1,
			       const char *s2, size_t n2) RD_UNUSED;
static inline int rd_strnncmp (const char *s1, size_t n1,
			       const char *s2, size_t n2) {
	int r;

	if ((r = strncmp(s1, s2, RD_MIN(n1, n2))))
		return r;

	return (int)n1-n2;
}


/**
 * Return the character position where s1 and s2 begin to differ.
 * If the strings are equal -1 will be returned.
 */
ssize_t rd_strdiffpos (const char *s1, const char *s2);
ssize_t rd_strndiffpos (const char *s1, size_t size1,
		       const char *s2, size_t size2);





/**
 * Frees thread-local resources on thread exit.
 */
void rd_string_thread_cleanup (void);






