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

#include "rdbits.h"
#include "rdqueue.h"
#include "rdthread.h"

/**
 * Single allocation, when RD_MEMCTX_F_TRACK is used.
 */
typedef struct rd_memctx_ptr_s {
	TAILQ_ENTRY(rd_memctx_ptr_s) rmcp_link;
	uint32_t      rmcp_size;
} rd_memctx_ptr_t;


/**
 * Memory context.
 */
typedef struct rd_memctx_s {
	TAILQ_ENTRY(rd_memctx_s) rmc_link;

	rd_mutex_t   rmc_lock;      /* For .._F_LOCK */

	char        *rmc_name;      /* Optional name */

	unsigned int rmc_out;       /* Number of current allocations */
	uint64_t     rmc_bytes_out; /* Sum of current allocations in bytes.
				     * Requires the use of rd_memctx_freesz()
				     * or RD_MEMCTX_F_TRACK. */

	int          rmc_flags;
#define RD_MEMCTX_F_TRACK   0x1    /* Track all allocations by maintaining
				    * a list of them.
				    * This allows the rd_memctx_freeall() call
				    * but makes rd_memctx_free() slower.
				    * Typical usage is for operations where
				    * a bunch of allocations are made
				    * and then freed all at once when the
				    * work is over. */
#define RD_MEMCTX_F_LOCK    0x2    /* This flag is required for memctx's
				    * shared by multiple threads to enable 
				    * mutex locking. */
#define RD_MEMCTX_F_INITED  0x100  /* Initialized */

	TAILQ_HEAD(, rd_memctx_ptr_s) rmc_ptrs;  /* If _F_TRACK is set:
						  * Current allocations. */
} rd_memctx_t;


#define RD_MEMCTX_LOCK(rmc)  do {				\
	if (BIT_TEST((rmc)->rmc_flags, RD_MEMCTX_F_LOCK))	\
		rd_mutex_lock(&(rmc)->rmc_lock);		\
	} while (0)

#define RD_MEMCTX_UNLOCK(rmc)  do {				\
	if (BIT_TEST((rmc)->rmc_flags, RD_MEMCTX_F_LOCK))	\
		rd_mutex_unlock(&(rmc)->rmc_lock);		\
	} while (0)

void rd_memctx_init (rd_memctx_t *rmc, const char *name, int flags);
void rd_memctx_destroy (rd_memctx_t *rmc);

#define RD_MEMCTX_INITED(rmc) ((rmc)->rmc_flags & RD_MEMCTX_F_INITED)

typedef struct rd_memctx_stats_s {
	unsigned int out;
	size_t       bytes_out;
} rd_memctx_stats_t;

static void rd_memctx_stats (rd_memctx_t *rmc, rd_memctx_stats_t *stats)
	RD_UNUSED;
static void rd_memctx_stats (rd_memctx_t *rmc, rd_memctx_stats_t *stats) {
	RD_MEMCTX_LOCK(rmc);
	stats->out = rmc->rmc_out;
	stats->bytes_out = rmc->rmc_bytes_out;
	RD_MEMCTX_UNLOCK(rmc);
}


typedef enum {
	RD_MEMCTX_MALLOC,
	RD_MEMCTX_CALLOC,
} rd_memctx_alloc_type_t;

void *rd_memctx_alloc (rd_memctx_t *rmc, size_t size,
		       rd_memctx_alloc_type_t type);

#define rd_memctx_malloc(rmc,size)  rd_memctx_alloc(rmc,size,RD_MEMCTX_MALLOC)
#define rd_memctx_calloc(rmc,nmemb,size)  \
	rd_memctx_alloc(rmc,(nmemb)*(size),RD_MEMCTX_CALLOC)

void  rd_memctx_free0 (rd_memctx_t *rmc, void *ptr, size_t size);
#define rd_memctx_free(rmc,ptr)       rd_memctx_free0(rmc,ptr,0);
#define rd_memctx_freesz(rmc,ptr,sz)  rd_memctx_free0(rmc,ptr,sz)

size_t rd_memctx_freeall (rd_memctx_t *rmc);

#define rd_memctx_name(rmc) ((rmc)->rmc_name)
