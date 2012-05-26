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

#include <rd.h>
#include <rdbuf.h>
#include <rdlog.h>

static int test_bufs (void) {
	rd_bufh_t *rbh;
	struct {
		int len;
		char *data;
	} bufs[] = {
		{ 16, "ABCDEFGHIJKLMNOP" },
		{ 4, "\n\ta\n" },
		{ 5, "bye\n\0" },
		{ 0, NULL },
	};
	int i;
	int totlen = 0;
	int fails = 0;
	char buf2[256];
	char *out;
	
	rbh = rd_bufh_new(NULL, 0);

	bufs[3].len = 512;
	bufs[3].data = alloca(bufs[3].len);
	for (i = 0 ; i < bufs[3].len ; i++)
		bufs[3].data[i] = i % 256;

	for (i = 0 ; i < RD_ARRAY_SIZE(bufs) ; i++) {
		totlen += bufs[i].len;
		rd_bufh_append(rbh, bufs[i].data, bufs[i].len, 0);
	}

	if (rd_bufh_len(rbh) != totlen) {
		printf("%s:%i: bufh length %i should be %i\n",
		       __FUNCTION__,__LINE__, rd_bufh_len(rbh), totlen);
		fails++;
	}

	rd_bufh_sprintf(rbh, "Hi there %s, you are #%i", "Mike", 8);

	out = alloca(rd_bufh_len(rbh)+1);
	out[rd_bufh_len(rbh)] = '\0';
	rd_bufh_copyout(rbh, out);

	/* We inserted a null-character in the bye-string, thats why
	 * we only match the beginning of the buffer. */
	if (strcmp(out, "ABCDEFGHIJKLMNOP\n\ta\nbye\n")) {
		printf("%s:%i: unexpected serialize#1 result: \"%s\"\n",
		       __FUNCTION__,__LINE__, out);
		fails++;
	}

	rd_bufh_destroy(rbh);


	/*
	 * Try small writes.
	 */
	rbh = rd_bufh_new(NULL, 0);
	for (i = 0 ; i < sizeof(buf2) ; i++) {
		buf2[i] = i % 256;
		rd_bufh_append(rbh, &buf2[i], 1, 0);
	}

	if (rd_bufh_len(rbh) != sizeof(buf2)) {
		printf("%s:%i: unexpected length for buf2: %i, should be %i\n",
		       __FUNCTION__,__LINE__, rd_bufh_len(rbh),
		       (int)sizeof(buf2));
		fails++;
	}

	out = alloca(rd_bufh_len(rbh));
	rd_bufh_copyout(rbh, out);

	if (memcmp(out, buf2, sizeof(buf2))) {
		printf("%s:%i: unexpected serialize buf2 result:\n",
		       __FUNCTION__,__LINE__);
		rd_hexdump(stdout, "serialized", out, rd_bufh_len(rbh));
		rd_hexdump(stdout, "buf2", buf2, sizeof(buf2));
		fails++;
	}
	
	rd_bufh_destroy(rbh);

	return fails;
}




int main (int argc, char **argv) {
	int fails = 0;

	rd_dbg_set(1);

	fails += test_bufs();

	return fails ? 1 : 0;
}
