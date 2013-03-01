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
#include "rdbits.h"

/**
 * FIXME: support for other bitvec types than static,
 *        a dynamicly growable and shrinkable would be nice,
 *        and a sparse one for larger spans.
 */

void rd_bitvec_free (rd_bitvec_t *rbv) {
	free(rbv->rbv_b);
}

void rd_bitvec_init (rd_bitvec_t *rbv, rd_bitvec_type_t type,
		     uint64_t max_val) {

	assert(type == RD_BITVEC_STATIC);

	memset(rbv, 0, sizeof(*rbv));

	rbv->rbv_max = max_val;

	/* FIXME: Proper tree-algo for larger maxes */
	assert(rbv->rbv_max <= 4096);
	rbv->rbv_buckets = 1 + (rbv->rbv_max / (8 * sizeof(*rbv->rbv_b)));
	rbv->rbv_b = calloc(rbv->rbv_buckets, sizeof(*rbv->rbv_b));
}

int rd_bitvec_op (rd_bitvec_t *rbv, uint64_t i, rd_bitvec_op_t op) {
	int bucket;
	int bit;

	if (i > rbv->rbv_max) {
		if (op == RD_BITVEC_OP_TEST)
			return 0;

		errno = ERANGE;
		return -1;
	}

	bucket = i / (8 * sizeof(*rbv->rbv_b));
	bit = 1 << (i % (8 * sizeof(*rbv->rbv_b)));

	switch (op)
	{
	case RD_BITVEC_OP_SET:
		rbv->rbv_b[bucket] |= bit;
		break;
	case RD_BITVEC_OP_RESET:
		rbv->rbv_b[bucket] &= ~bit;
		break;
	case RD_BITVEC_OP_TEST:
		return rbv->rbv_b[bucket] & bit;
	default:
		errno = EINVAL;
		return -1;
	}

	return 0;
}

/**
 * rd_bitvec_ffs() and rd_bitvec_fls() returns the bit number
 * according to ffs(3).
 */
uint64_t rd_bitvec_fxs (const rd_bitvec_t *rbv, rd_bitvec_op_t op) {
	int bucket = 0;
	uint32_t bit = 0;

	if (op == RD_BITVEC_OP_FFS) {
		for (bucket = 0 ; bucket < rbv->rbv_buckets ; bucket++)
			if (rbv->rbv_b[bucket] &&
			    (bit = ffs(rbv->rbv_b[bucket])))
				break;

	} else if (op == RD_BITVEC_OP_FLS) {
		for (bucket = rbv->rbv_buckets - 1 ; bucket >= 0 ; bucket--) {
			if (rbv->rbv_b[bucket] &&
			    (bit = __builtin_clz(rbv->rbv_b[bucket]))) {
				bit = (8 * sizeof(*rbv->rbv_b)) - bit;
				break;
			}
		}


	} else {
		assert(!*"unknown RD_BITVEC_OP_..");
	}

	return (uint64_t)bit + (uint64_t)(bucket * (8 * sizeof(*rbv->rbv_b)));
}
