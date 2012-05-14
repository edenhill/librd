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

#pragma once

#include <string.h>
#include <stdarg.h>


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
 * Frees thread-local resources on thread exit.
 */
void rd_string_thread_cleanup (void);
