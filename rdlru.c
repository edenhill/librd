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
#include "rdlru.h"


static void rd_lru_elm_destroy (rd_lru_t *rlru, rd_lru_elm_t *rlrue) {
	if (rlru)
		TAILQ_REMOVE(&rlru->rlru_elms, rlrue, rlrue_link);
	free(rlrue);
}


void rd_lru_destroy (rd_lru_t *rlru) {
	rd_lru_elm_t *rlrue;

	while ((rlrue = TAILQ_FIRST(&rlru->rlru_elms)))
		rd_lru_elm_destroy(rlru, rlrue);

	free(rlru);
}


rd_lru_t *rd_lru_new (void) {
	rd_lru_t *rlru;

	rlru = calloc(1, sizeof(*rlru));

	TAILQ_INIT(&rlru->rlru_elms);

	return rlru;
}


void rd_lru_push (rd_lru_t *rlru, void *ptr) {
	rd_lru_elm_t *rlrue;

	rlrue = calloc(1, sizeof(*rlrue));
	rlrue->rlrue_ptr = ptr;

	TAILQ_INSERT_HEAD(&rlru->rlru_elms, rlrue, rlrue_link);
	rlru->rlru_cnt++;
}


void *rd_lru_pop (rd_lru_t *rlru) {
	rd_lru_elm_t *rlrue;
	void *ptr;

	if ((rlrue = TAILQ_LAST(&rlru->rlru_elms, rd_lru_elm_head))) {
		ptr = rlrue->rlrue_ptr;
		rd_lru_elm_destroy(rlru, rlrue);
	} else
		ptr = NULL;

	return ptr;
}


void *rd_lru_shift (rd_lru_t *rlru) {
	rd_lru_elm_t *rlrue;
	void *ptr;

	if ((rlrue = TAILQ_FIRST(&rlru->rlru_elms))) {
		ptr = rlrue->rlrue_ptr;
		rd_lru_elm_destroy(rlru, rlrue);
	} else
		ptr = NULL;

	return ptr;
}
