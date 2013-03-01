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
#include "rdfile.h"

#include "rdtests.h"

static int test_read_write (void) {
	TEST_VARS;
	const char *path = "__tmp_test_0003";
	int r;
	char buf[] =
		"Line1 is 22 bytes long\n"
		"Line2 is just 16\n";
	int blen = strlen(buf);
	int len;
	char *tmp;

	r = rd_file_write(path, buf, blen, O_TRUNC, 0600);
	if (r == -1) {
		unlink(path);
		TEST_FAIL_RETURN("file write: %s", strerror(errno));
	}

	tmp = rd_file_read(path, &len);
	if (!tmp) {
		unlink(path);
		TEST_FAIL_RETURN("file read: %s", strerror(errno));
	}

	if (len != blen)
		TEST_FAIL("file read: written len %i != read len %i",
			  blen, len);

	free(tmp);

	unlink(path);

	TEST_RETURN;
}


int main (int argc, char **argv) {
	TEST_VARS;
	const char *path1 = "/abra/cadabra/bot";
	const char *name1 = "bot";
	const char *path2 = "/abra/camambra/bark/";
	const char *name2 = "";
	const char *path3 = "/";
	const char *name3 = "";

	rd_init();

	fails += strcmp(rd_basename(path1), name1);
	fails += strcmp(rd_basename(path2), name2);
	fails += strcmp(rd_basename(path3), name3);
	if (fails)
		TEST_FAIL("rd_basename tests failed");

	fails += test_read_write();

	TEST_EXIT;
}
