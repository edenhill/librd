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



#define RD_MEM_END_TOKEN -2

/**
 * rd_calloc_struct() provides a dynamic struct initializer with only one
 * single calloc call.
 *
 * The key benefits of this is:
 *   - higher performance thanks to fewer malloc/free calls
 *     and the possibility that all data of the struct resides
 *     within one cache line
 *   - less memory fragmentation
 *   - simpler cleanup code (one free() call).
 *
 *
 * Allocates memory for a struct of base size `base_size' +
 * the sum of all memory tuples (length, srcptr, dstptr) following in the
 * var-args list. The srcptrs are then copied to their new locations
 * and dstptr is updated to point to the newly allocated memory.
 *
 * The tuple memory is fitted to the end of the struct.
 * If tuple length is -1 then srcptr is assumed to be a null-terminated
 * string which is automatically strlen():ed.
 *
 * The tuple argument list must be terminated with a single RD_MEM_END_TOKEN.
 *
 * Example usage:
 *
 *   struct mystruct {
 *       char *name;    (1)
 *       int   id;
 *       char  token[6];
 *       void *data;    (1)
 *       int   data_len;
 *       (2)
 *   }
 *   ...
 *   struct mystruct *st;
 *
 *   rd_calloc_struct(&st, sizeof(*st),
 *                    -1, "My name", &st->name,
 *                    buf->len, buf->ptr, &st->data,
 *                    RD_MEM_END_TOKEN);
 *
 *   st->data_len = buf->len;
 *   st->....
 *
 *
 *  When done:
 *     free(st);
 * 
 *
 *  (1) = 'name' and 'data' fields will be allocated space for at (2)
 *        and the two pointers will be updated to point the (2) data space.
 *  (2) = space allocated for initialized fields.
 */
#define rd_calloc_struct(pptr,base_size,ARGS...) do {			\
		*(pptr) = NULL;						\
		*(pptr) = rd_calloc_struct0(NULL, base_size, ARGS);	\
	} while (0)

/**
 * rd_calloc_struct() for memctx use.
 */
#define rd_memctx_calloc_struct(rmc,pptr,base_size,ARGS...) do {	\
		*(pptr) = NULL;						\
		*(pptr) = rd_calloc_struct0(rmc, base_size, ARGS);	\
	} while (0)
void *rd_calloc_struct0 (rd_memctx_t *rmc, size_t base_size, ...);






/**
 * Allocate and copy memory from src to dst
 */
static void *rd_memdup (const void *src, size_t len) RD_UNUSED;
static void *rd_memdup (const void *src, size_t len) {
	void *dst;

	dst = malloc(len);
	if (likely(dst != NULL))
		memcpy(dst, src, len);

	return dst;
}



/**
 * Generic struct to hold a pointer and its length.
 */
typedef struct rd_ptrlen_s {
	char  *ptr;   /* char rather than void here allows byte-sized
		       * pointer arithmetics, which is handy. */
	size_t len;
} rd_ptrlen_t;
