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

#include "rdthread.h"
#include "rdsysqueue.h"



typedef struct rd_lru_elm_s {
	TAILQ_ENTRY(rd_lru_elm_s)  rlrue_link;
	void       *rlrue_ptr;
} rd_lru_elm_t;

TAILQ_HEAD(rd_lru_elm_head, rd_lru_elm_s);


typedef struct rd_lru_s {
	rd_mutex_t  rlru_lock;
	rd_cond_t   rlru_cond;
	int         rlru_cnt;
	struct rd_lru_elm_head rlru_elms;
} rd_lru_t;


#define RD_LRU_INITIALIZER(st) \
	{ .rlru_elms = TAILQ_HEAD_INITIALIZER(st.rlru_elms) }

#define rd_lru_lock(rlru)   rd_mutex_lock(&(rlru)->rlru_lock)
#define rd_lru_unlock(rlru) rd_mutex_unlock(&(rlru)->rlru_lock)


/**
 * Destroys an LRU
 */
void rd_lru_destroy (rd_lru_t *rlru);


/**
 * Creates a new LRU
 */
rd_lru_t *rd_lru_new (void);


/**
 * Push a new entry to the LRU.
 */
void rd_lru_push (rd_lru_t *rlru, void *ptr);

/**
 * Returns and removes the oldest entry in the LRU, or NULL.
 */
void *rd_lru_pop (rd_lru_t *rlru);

/**
 * Returns and removes the youngest entry in the LRU, or NULL.
 */
void *rd_lru_shift (rd_lru_t *rlru);

/**
 * Returns the number of entries in the LRU
 */
#define rd_lru_cnt(rlru) ((rlru)->rlru_cnt)
