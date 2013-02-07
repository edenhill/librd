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
#include "rdencoding.h"
#include "rdlog.h"

#include "rdtests.h"

static int test_varint (void) {
	TEST_VARS;
	char buf[24];
	int r;
	int vlen;
	uint64_t u64is[] = { 0, 12345678910112131415llu, 300 };
	uint64_t s64is[] = { 0, -12345678910112131415llu,
			     481924891llu -300, 300 };
	uint64_t v, v2;
	int i;

	/* Unsigned */
	for (i = 0 ; i < RD_ARRAY_SIZE(u64is) ; i++) {
		if ((r = rd_varint_encode_u64(u64is[i],
					      buf, sizeof(buf))) == -1)
			TEST_FAIL("#%i: varint encode failed", i);

		v = rd_varint_decode_u64(buf, sizeof(buf), &vlen);
		if (vlen <= 0)
			TEST_FAIL("#%i: varint decode failed: vlen = %i",
				  i, vlen);
		if (u64is[i] != v)
			TEST_FAIL("#%i: varint decode incorrect: "
				  "in %"PRIx64 " != out %"PRIx64,
			  i, u64is[i], v);
	}

	/* Signed */
	for (i = 0 ; i < RD_ARRAY_SIZE(s64is) ; i++) {
		if ((r = rd_varint_encode_s64(s64is[i],
					      buf, sizeof(buf))) == -1)
			TEST_FAIL("#%i: varint encode failed", i);

		v = rd_varint_decode_s64(buf, sizeof(buf), &vlen);
		if (vlen <= 0)
			TEST_FAIL("#%i: varint decode failed: vlen = %i",
				  i, vlen);
		if (s64is[i] != v)
			TEST_FAIL("#%i: varint decode incorrect: "
				  "in %"PRId64 " != out %"PRId64,
			  i, s64is[i], v);
	}

	/* Reference buffer (300) */
	v2 = 300llu;
	buf[0] = 0xac;
	buf[1] = 0x02;
	buf[2] = 0xff;
	v = rd_varint_decode_u64(buf, sizeof(buf), &vlen);
	if (RD_VARINT_DECODE_ERR(vlen))
		TEST_FAIL("#%i: varint decode failed: vlen = %i",
			  i, vlen);
	if (v2 != v)
		TEST_FAIL("#%i: varint decode incorrect: "
			  "in %"PRId64 " != out %"PRId64,
			  i, v2, v);

	/* Buffer under flow */
	v = rd_varint_decode_u64(buf, 1, &vlen);
	TEST_ASSERT(RD_VARINT_DECODE_ERR(vlen));
	TEST_ASSERT(RD_VARINT_DECODE_UNDERFLOW(vlen));


	TEST_RETURN;
}


static int test_hex (void) {
	TEST_VARS;
	const char *in = "My hex testing string is here, 123456789\0notincl";
	char tmp1[512], tmp2[256];
	char bin[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
			 0x11, 0x3f, 0x7f, 0xf1, 0x44, 0xff };
	int r;

	rd_bin2hex(in, strlen(in)+1, tmp1, sizeof(tmp1));
	rd_hex2bin(tmp1, -1, tmp2, sizeof(tmp2));
	TEST_STR_EQ(tmp2, in);

	rd_bin2hex(bin, sizeof(bin), tmp1, sizeof(tmp1));
	TEST_STR_EQ(tmp1, "00010203040506070809113f7ff144ff");
	r = rd_hex2bin(tmp1, -1, tmp2, sizeof(tmp2));
	TEST_INT_EQ(r, 16);
	TEST_ASSERT(!memcmp(tmp2, bin, 16));

	TEST_RETURN;
}

int main (int argc, char **argv) {
	TEST_VARS;

	TEST_INIT;

	fails += test_varint();
	fails += test_hex();

	TEST_EXIT;
}

