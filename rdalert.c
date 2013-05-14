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

#include "rd.h"
#include "rdsysqueue.h"
#include "rdthread.h"
#include "rdalert.h"
#include "rdlog.h"
#include <stdarg.h>

struct rd_alert_cb {
	TAILQ_ENTRY(rd_alert_cb) link;
	rd_alert_type_t type;
	void (*callback) (rd_alert_type_t type, int level,
			  const char *reason, void *opaque, ...);
	void *opaque;
};

static TAILQ_HEAD(, rd_alert_cb) rd_alert_cbs =
	TAILQ_HEAD_INITIALIZER(rd_alert_cbs);
static rd_mutex_t rd_alert_lock;

static const char *rd_alert_names[] = {
	"THREAD_STALL"
};

void rd_alert0 (const char *file, const char *func, int line,
		rd_alert_type_t type, int level, const char *reason, ...) {
	va_list ap;
	struct rd_alert_cb *rac;

	rdbg("%%%i: %s ALERT at %s:%i:%s: %s",
	     level, rd_alert_names[type], file, line, func, reason);

	rd_mutex_lock(&rd_alert_lock);
	va_start(ap, reason);
	TAILQ_FOREACH(rac, &rd_alert_cbs, link) {
		if (rac->type == type || rac->type == RD_ALERT_ALL) {
			va_list ap2;
			va_copy(ap2, ap);
			rac->callback(type, level, reason, rac->opaque, ap2);
		}
	}
	va_end(ap);
	rd_mutex_unlock(&rd_alert_lock);
}



void rd_alert_register (rd_alert_type_t type,
			void (*callback) (rd_alert_type_t type, int level,
					  const char *reason, void *opaque,
					  ...),
			void *opaque) {
	struct rd_alert_cb *rac;
	
	rac = calloc(1, sizeof(*rac));
	rac->type     = type;
	rac->callback = callback;
	rac->opaque   = opaque;

	rd_mutex_lock(&rd_alert_lock);
	TAILQ_INSERT_TAIL(&rd_alert_cbs, rac, link);
	rd_mutex_unlock(&rd_alert_lock);
}

