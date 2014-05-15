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

#include <syslog.h>
#include <stdio.h>

void rdputs0 (const char *file, const char *func, int line,
	      int severity,const char *fmt, ...)
	__attribute__((format (printf, 5, 6)));

#define rdbg(fmt...) \
	rdputs0(__FILE__,__FUNCTION__,__LINE__,LOG_DEBUG,fmt)
#define rdlog(severity,fmt...) \
	rdputs0(__FILE__,__FUNCTION__,__LINE__,severity,fmt)

void rd_dbg_ctx_push (const char *fmt, ...);
void rd_dbg_ctx_pop (void);
void rd_dbg_ctx_clear (void);
void rd_log_set_severity (int severity);

#define rd_dbg_set(onoff) rd_dbg_set_severity(onoff ? LOG_DEBUG : LOG_INFO)

void rd_hexdump (FILE *fp, const char *name, const void *ptr, size_t len);
