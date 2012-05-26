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
#include "rdstring.h"
#include "rdmem.h"

/*
 * Thread-local states
 */

struct rdstr_cyclic {
	int    size;
	char **buf;
	int   *len;
	int    i;
};

static __thread struct {
	
	/* rd_tsprintf() states */
	struct rdstr_cyclic tsp;
} rdstr_states;


/**
 * Initializes a cyclic state or updates it to the next index.
 */
static inline struct rdstr_cyclic *rdstr_cyclic_get (struct rdstr_cyclic *cyc,
						     int size) {
	
	if (unlikely(cyc->size == 0)) {
		/* Not initialized. */

		/* Allocate cyclic buffer array. */
		cyc->size = size;
		cyc->buf = calloc(sizeof(*cyc->buf), cyc->size);
		cyc->len = calloc(sizeof(*cyc->len), cyc->size);
	} else /* Update to the next pointer. */
		cyc->i = (cyc->i + 1) % cyc->size;

	return cyc;
}



char *rd_tsprintf (const char *format, ...) {
	va_list ap;
	int len;
	struct rdstr_cyclic *cyc;

	cyc = rdstr_cyclic_get(&rdstr_states.tsp, RD_TSPRINTF_BUFCNT);

	va_start(ap, format);
	len = vsnprintf(NULL, 0, format, ap);
	va_end(ap);

	if (len < 0) /* Error */
		return NULL;

	len++; /* Include nul-byte */

	if (cyc->buf[cyc->i] == NULL ||
	    cyc->len[cyc->i] < len ||
	    (cyc->len[cyc->i] > (len * 4) &&
	     cyc->len[cyc->i] > 64)) {
		if (cyc->buf[cyc->i])
			free(cyc->buf[cyc->i]);

		cyc->len[cyc->i] = len;
		cyc->buf[cyc->i] = malloc(len);
	}


	va_start(ap, format);
	vsnprintf(cyc->buf[cyc->i], cyc->len[cyc->i], format, ap);
	va_end(ap);

	return cyc->buf[cyc->i];
}


int rd_snprintf_cat (char *str, size_t size, const char *format, ...) {
	va_list ap;
	int of;

	of = strlen(str);

	if (of >= size) {
		errno = ERANGE;
		return -1;
	}

	va_start(ap, format);
	of += vsnprintf(str+of, size-of, format, ap);
	va_end(ap);
	
	return of;
}


void rd_string_thread_cleanup (void) {
	int i;
	struct rdstr_cyclic *cyc;

	/* rd_tsprintf() states */
	cyc = &rdstr_states.tsp;
	if (cyc->size) {
		for (i = 0 ; i < cyc->size ; i++)
			if (cyc->buf[i])
				free(cyc->buf[i]);

		free(cyc->buf);
		free(cyc->len);
		cyc->size = 0;
	}
}
