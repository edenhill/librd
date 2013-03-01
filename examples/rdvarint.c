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


/* Typical include path would be <librd/rd..h>, but this program
 * is builtin from within the librd source tree and thus differs. */
#include "rd.h"       /* librd base */
#include "rdlog.h"
#include "rdencoding.h"

int main (int argc, char **argv) {
	uint64_t u64;
	char buf[16];
	int len, vlen;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s {encode|decode} <val>\n", argv[0]);
		exit(1);
	}


	if (argv[1][0] == 'e') {
		u64 = strtoull(argv[2], NULL, 0);
		len = rd_varint_encode_u64(u64, buf, sizeof(buf));
		printf("encoded u64 %"PRIu64" to:\n", u64);
		rd_hexdump(stdout, "hexdump", buf, len);
	} else {
		len = rd_hex2bin(argv[2], -1, buf, sizeof(buf));
		u64 = rd_varint_decode_u64(buf, len, &vlen);
		printf("decoded %s to vlen %i: %"PRIu64 "\n",
		       argv[2], vlen, u64);
		if (vlen < 0)
			printf("Error: overflow\n");
		else if  (vlen == 0)
			printf("Error: buffer was empty\n");
	}
	return 0;
}
