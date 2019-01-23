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
#include "rdmem.h"
#include "rdthread.h"
#include "rdbits.h"

#include <stdarg.h>

static rd_mutex_t rd_memctxs_lock = RD_MUTEX_INITIALIZER;
static TAILQ_HEAD(, rd_memctx_s) rd_memctxs =
	TAILQ_HEAD_INITIALIZER(rd_memctxs);



void rd_memctx_init (rd_memctx_t *rmc, const char *name, int flags) {

	memset(rmc, 0, sizeof(*rmc));

	rmc->rmc_flags = flags;
	if (name)
		rmc->rmc_name = strdup(name);

	if (BIT_TEST(rmc->rmc_flags, RD_MEMCTX_F_LOCK))
		rd_mutex_init(&rmc->rmc_lock);

	if (BIT_TEST(rmc->rmc_flags, RD_MEMCTX_F_TRACK))
		TAILQ_INIT(&rmc->rmc_ptrs);

	rd_mutex_lock(&rd_memctxs_lock);
	TAILQ_INSERT_TAIL(&rd_memctxs, rmc, rmc_link);
	rd_mutex_unlock(&rd_memctxs_lock);

	BIT_SET(rmc->rmc_flags, RD_MEMCTX_F_INITED);
}

void rd_memctx_destroy (rd_memctx_t *rmc) {

	rd_mutex_lock(&rd_memctxs_lock);
	TAILQ_REMOVE(&rd_memctxs, rmc, rmc_link);
	rd_mutex_unlock(&rd_memctxs_lock);

	if (BIT_TEST(rmc->rmc_flags, RD_MEMCTX_F_TRACK))
		rd_memctx_freeall(rmc);

	if (rmc->rmc_name)
		free(rmc->rmc_name);
}


/**
 * RD_MEMCTX_LOCK must be held.
 */
static void rd_memctx_ptr_free (rd_memctx_t *rmc, rd_memctx_ptr_t *rmcp) {

	assert(rmc->rmc_out > 0);
	rmc->rmc_out--;

	if (likely(rmcp->rmcp_size)) {
		assert(rmc->rmc_bytes_out > 0);
		rmc->rmc_bytes_out -= rmcp->rmcp_size;
	}

	TAILQ_REMOVE(&rmc->rmc_ptrs, rmcp, rmcp_link);

	free(rmcp);
}

static inline void *rd_memctx_alloc0 (size_t size,
				      rd_memctx_alloc_type_t type) {
	switch (type)
	{
	case RD_MEMCTX_MALLOC:
		return malloc(size);
	case RD_MEMCTX_CALLOC:
		return calloc(1, size);
		break;
	}
	assert(!*"notreached");
	return NULL;
}


/**
 * RD_MEMCTX_LOCK must be held
 */
static rd_memctx_ptr_t *rd_memctx_ptr_new (rd_memctx_t *rmc,
					   size_t size, void **ptr,
					   rd_memctx_alloc_type_t type) {
	rd_memctx_ptr_t *rmcp;

	rmcp = rd_memctx_alloc0(sizeof(*rmcp) + size, type);
	
	rmcp->rmcp_size = size;
	*ptr = rmcp+1;

	TAILQ_INSERT_TAIL(&rmc->rmc_ptrs, rmcp, rmcp_link);

	return rmcp;
}



void *rd_memctx_alloc (rd_memctx_t *rmc, size_t size,
		       rd_memctx_alloc_type_t type) {
	void *ptr;

	RD_MEMCTX_LOCK(rmc);

	if (BIT_TEST(rmc->rmc_flags, RD_MEMCTX_F_TRACK))
		rd_memctx_ptr_new(rmc, size, &ptr, type);
	else {
		switch (type)
		{
		case RD_MEMCTX_MALLOC:
			ptr = malloc(size);
			break;
		case RD_MEMCTX_CALLOC:
			ptr = calloc(1, size);
			break;
		default:
			assert(!*"unknown alloc type");
			break;
		}
	}

	rmc->rmc_out++;
	rmc->rmc_bytes_out += size;

	RD_MEMCTX_UNLOCK(rmc);
	return ptr;
}


void rd_memctx_free0 (rd_memctx_t *rmc, void *ptr, size_t size) {

	RD_MEMCTX_LOCK(rmc);

	if (size) {
		assert(rmc->rmc_bytes_out - size >= 0);
		rmc->rmc_bytes_out -= size;
	}

	assert(rmc->rmc_out > 0);
	rmc->rmc_out--;
	if (BIT_TEST(rmc->rmc_flags, RD_MEMCTX_F_TRACK)) {
		rd_memctx_ptr_t *rmcp = (rd_memctx_ptr_t *)ptr - 1;
		TAILQ_REMOVE(&rmc->rmc_ptrs, rmcp, rmcp_link);
		ptr = rmcp;
	}
	
	free(ptr);
	

	RD_MEMCTX_UNLOCK(rmc);
}



/**
 * Frees all memory allocated with the memctx and returns the
 * number of bytes freed.
 * Requires RD_MEMCTX_F_TRACK to be set.
 */
size_t rd_memctx_freeall (rd_memctx_t *rmc) {
	rd_memctx_ptr_t *rmcp;
	size_t sum = 0;

	assert(BIT_TEST(rmc->rmc_flags, RD_MEMCTX_F_TRACK));

	RD_MEMCTX_LOCK(rmc);

	while ((rmcp = TAILQ_FIRST(&rmc->rmc_ptrs))) {
		sum += rmcp->rmcp_size;
		rd_memctx_ptr_free(rmc, rmcp);
	}

	RD_MEMCTX_UNLOCK(rmc);
	
	return sum;
}



void *rd_calloc_struct0 (rd_memctx_t *rmc, size_t base_size, ...) {
	va_list ap;
	size_t tot_size = base_size;
	int pass;
	char *tail = NULL;
	void *ptr = NULL;

	/*
	 * 1) Pass 1: calculate total memory size.
	 * 2) Allocate memory 
	 * 3) Pass 2: copy elements to allocated memory.
	 * 4) Return
	 */

	for (pass = 1 ; pass <= 2 ; pass++) {
		int elem_size;
		void *elem_src, **elem_dst;

		va_start(ap, base_size);
		while ((elem_size = va_arg(ap, int)) != RD_MEM_END_TOKEN) {
			elem_src = va_arg(ap, void *);
			elem_dst = va_arg(ap, void **);

			if (elem_size == -1) /* String of unknown length. */
				elem_size = strlen(elem_src) + 1;

			if (pass == 1)
				tot_size += elem_size;
			else {
				elem_dst = (void *)((char *)ptr +
						    (size_t)(elem_dst));
				*elem_dst = tail;
				memcpy(*elem_dst, elem_src, elem_size);
				tail += elem_size;
			}
		}
		va_end(ap);

		if (pass == 1) {
			if (rmc)
				ptr = rd_memctx_alloc(rmc, tot_size,
						      RD_MEMCTX_CALLOC);
			else
				ptr = calloc(1, tot_size);

			tail = ((char *)ptr) + base_size;

			if (tot_size == base_size)
				break;
		}
	}

	return ptr;
}








#if 0
/**
 * Returns memory information about the designated process.
 */
size_t rd_mem_proc (pid_t pid) {
	char path[64];
	char buf[128];
	int fd;

	sprintf(path, "/proc/%i/statm", pid);
	if ((fd = open(path, O_RDONLY) == -1))
		return 0;

	if (read(fd, buf, sizeof(buf)) < 1) {
		close(fd);
		return 0;
	}

	/* FIXME: scanf and so on...
	 *        produce a struct that includes the relevant fields.
	 * http://elinux.org/Runtime_Memory_Measurement
	 *
	 * Then make rd_mem_rss() that returns just the rss field.
	 */
}
#endif
