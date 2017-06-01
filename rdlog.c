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

#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "rd.h"
#include "rdthread.h"
#include "rdlog.h"




#define RD_DBG_CTXS_MAX 32
static __thread char *rd_dbg_ctxs[RD_DBG_CTXS_MAX];
static __thread int rd_dbg_ctx_idx = 0;
static __thread int rd_dbg_ctx_wanted_idx = 0;

static int rd_current_severity = LOG_INFO;

void rd_log_set_severity (int severity) {
	rd_current_severity = severity;
}


void rd_dbg_ctx_push (const char *fmt, ...) {
	va_list ap;
	char *buf;

	rd_dbg_ctx_wanted_idx++;

	if (rd_dbg_ctx_idx + 1 == RD_DBG_CTXS_MAX)
		return;

	buf = malloc(64);

	va_start(ap, fmt);
	vsnprintf(buf, 64, "%s", ap);
	va_end(ap);

	rd_dbg_ctx_idx++;
}

void rd_dbg_ctx_pop (void) {
	assert(rd_dbg_ctx_wanted_idx-- > 0);
	assert(rd_dbg_ctx_idx-- > 0);
	free(rd_dbg_ctxs[rd_dbg_ctx_idx]);
}

void rd_dbg_ctx_clear (void) {
	while (rd_dbg_ctx_idx > 0)
		rd_dbg_ctx_pop();
}



void rdputs0 (const char *file, const char *func, int line,
		int severity, const char *fmt, ...) {
	va_list ap;
	char buf[4096];
	int of = 0;
	int i;
	int r RD_UNUSED;
	rd_ts_t now;
	static __thread char thrname[16];
	int printf_rc;

	if (severity > rd_current_severity)
		return;

	now = rd_clock();

	if (unlikely(!rd_currthread && !*thrname)) {
		const pthread_t tid = pthread_self();
		char *cursor = thrname;
		int of = sprintf(cursor, "thr:");

		for (i=0; i<sizeof(tid) && of < sizeof(thrname);
		    ++i) {
			cursor += snprintf(cursor, sizeof(thrname), "%02x",
					   ((uint8_t *)&tid)[i]);
		}
	}

	of += snprintf(buf+of, sizeof(buf)-of,
		       "|%"PRIu64".%06"PRIu64"|%s:%i|%s| ",
		       now / (uint64_t)1000000,
		       now % (uint64_t)1000000,
		       func, line,
		       rd_currthread ? rd_currthread->rdt_name : thrname);

	if (rd_dbg_ctx_idx > 0) {
		for (i = 0 ; i < rd_dbg_ctx_idx ; i++)
			of += snprintf(buf+of, sizeof(buf)-of, "%s[%s]",
				       i ? "->" : "",
				       rd_dbg_ctxs[i]);

		of += snprintf(buf+of, sizeof(buf)-of, " ");
	}

	va_start(ap, fmt);
	printf_rc = vsnprintf(buf+of, sizeof(buf)-of, fmt, ap);
	va_end(ap);

	if( printf_rc > sizeof(buf) - of ) {
		// Should we log a log buffer overflow? should we care about 
		// log overflow log overflows?
		of = sizeof(buf)-2;
		buf[of-1] = buf[of-2] = buf[of-3] = '.';
	} else {
		of += printf_rc;
	}

	buf[of++] = '\n';
	buf[of] = '\0';

	r = write(STDOUT_FILENO, buf, of);
}





void rd_hexdump (FILE *fp, const char *name, const void *ptr, size_t len) {
	const char *p = (const char *)ptr;
	size_t of = 0;


	if (name)
		fprintf(fp, "%s hexdump (%zu bytes):\n", name, len);

	for (of = 0 ; of < len ; of += 16) {
		char hexen[16*3+1];
		char charen[16+1];
		int hof = 0;

		int cof = 0;
		int i;

		for (i = of ; i < of + 16 && i < len ; i++) {
			hof += sprintf(hexen+hof, "%02x ", p[i] & 0xff);
			cof += sprintf(charen+cof, "%c",
				      isprint(p[i]) ? p[i] : '.');
		}
		fprintf(fp, "%08zu: %-48s %-16s\n",
			of, hexen, charen);
	}
}
