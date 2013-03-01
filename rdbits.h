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

#define BIT_SET(f,b)    ((f) |= (b))
#define BIT_RESET(f,b)  ((f) &= ~(b))
#define BIT_TEST(f,b)   ((f) & (b))
#define BIT_MATCH(f,b)  (((f) & (b)) == (b))


#define rd_assert_bit(VAR,BIT)  assert(((VAR) & (BIT)))
#define rd_assert_nbit(VAR,BIT) assert(!((VAR) & (BIT)))


/**
 * Bit vectors
 */

typedef enum {
	RD_BITVEC_STATIC,  /* Vector does not grow, allocated on init. */
} rd_bitvec_type_t;

typedef struct rd_bitvec_s {
	uint64_t  rbv_max;
	uint64_t  rbv_size;
	int       rbv_buckets;
	uint32_t *rbv_b;
} rd_bitvec_t;

void rd_bitvec_init (rd_bitvec_t *rbv, rd_bitvec_type_t type,
		     uint64_t max_val);
void rd_bitvec_free (rd_bitvec_t *rbv);

typedef enum {
	RD_BITVEC_OP_SET,
	RD_BITVEC_OP_RESET,
	RD_BITVEC_OP_TEST,
	RD_BITVEC_OP_FFS,
	RD_BITVEC_OP_FLS
} rd_bitvec_op_t;

int rd_bitvec_op (rd_bitvec_t *rbv, uint64_t i, rd_bitvec_op_t op);

#define rd_bitvec_set(rbv,i)   rd_bitvec_op(rbv,i,RD_BITVEC_OP_SET)
#define rd_bitvec_reset(rbv,i) rd_bitvec_op(rbv,i,RD_BITVEC_OP_RESET)
#define rd_bitvec_test(rbv,i)  rd_bitvec_op(rbv,i,RD_BITVEC_OP_TEST)


uint64_t rd_bitvec_fxs (const rd_bitvec_t *rbv, rd_bitvec_op_t op);
#define rd_bitvec_ffs(rbv)     rd_bitvec_fxs(rbv,RD_BITVEC_OP_FFS)
#define rd_bitvec_fls(rbv)     rd_bitvec_fxs(rbv,RD_BITVEC_OP_FLS)
