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
#include "rdunits.h"


/**
 * Returns a thread-local temporary usable string comprising
 * the abbreviated string representation of an integer number.
 * Either according to the SI units (where kilo means 1000) when
 * si=1, or by the IEC units (where kilo means 1024) when si=0.
 * An optional unitsuffix may be specified, e.g., "B" for bytes.
 *
 * Abbreviated numbers will be shown at a precision of 2 decimals.
 *
 * Thread-safe.
 */
const char *rd_size2str (uint64_t size, int si, const char *unitsuffix) {
	static __thread char ret[16][40];
	static __thread int  reti = 0;
	static const struct {
		char    *suffix;
		uint64_t val;
	} sizes[2][6] =
		  {
			  [0] = { /* IEC units (i.e., 2^20 for MiB) */
				  { "Pi", RD_SIZE_PIB(1) },
				  { "Ti", RD_SIZE_TIB(1) },
				  { "Gi", RD_SIZE_GIB(1) },
				  { "Mi", RD_SIZE_MIB(1) },
				  { "Ki", RD_SIZE_KIB(1) },
				  { "", 0 },
			  },
			  [1] = { /* SI units (i.e., 10^6 for MB) */
				  { "P", 1000000000000000LLU },
				  { "T", 1000000000000LLU },
				  { "G", 1000000000LLU },
				  { "M", 1000000LLU },
				  { "K", 1000LLU },
				  { "", 0 },
			  },
		  };
	uint64_t sub;
	int i = -1;

	reti = (reti + 1) % 16;

	if (size == 0LLU) {
		/* Quick case for zero. */
		snprintf(ret[reti], sizeof(ret[reti])
			 , "0%s", unitsuffix ? : "");
		return ret[reti];
	}
					   

	si = !!si;
	
	/* Find proper size */
	while (sizes[si][++i].val > size)
		;

	/* Display according to size-abbreviation and precision */
	if (size < sizes[si][i].val || sizes[si][i].val == 0)
		snprintf(ret[reti], sizeof(ret[reti]),
			 "%" PRIu64 "%s%s",
			 size,
			 sizes[si][i].suffix, unitsuffix ? : "");
	else if (!(sub = size % sizes[si][i].val) ||
		 (float)sub / (float)sizes[si][i].val < 0.01)
		snprintf(ret[reti], sizeof(ret[reti]),
			 "%" PRIu64 "%s%s",
			 sizes[si][i].val ? size / sizes[si][i].val : size,
			 sizes[si][i].suffix, unitsuffix ? : "");

	else
		snprintf(ret[reti], sizeof(ret[reti]),
			 "%.2f%s%s",
			 (double)size / (double)sizes[si][i].val,
			 sizes[si][i].suffix, unitsuffix ? : "");

	return ret[reti];
}
	

