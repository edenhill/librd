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
#include <rdstring.h>

static int test_string (void) {
	char *str1 = "1234\n5678";
	char *str2;
	const char *delimiters = "14\n8";
	char *match[] = { str1, str1+3, str1+4, str1+8,
			  str1+strlen(str1) };
	const char *d = delimiters;
	char *m;
	int i;
	int fails = 0;

	for (i = 0 ; i < RD_ARRAY_SIZE(match) ; i++) {
		m = match[i];

		char *t;
		t = rd_strnchrs(str1, -1, d, 1);
		if (t != m) {
			printf("%s:%i: rd_strnchrs#1:%i failed: "
			       "delimiters '%s': expected '%s' but got '%s'\n",
			       __FUNCTION__,__LINE__, i, d, m, t);
			fails++;
			break;
		}
		d++;
	}

	str2 = "123456789";
	/* Look for something outside the buffer. */
	if ((m = rd_strnchrs(str2, 3, "4", 0))) {
		printf("%s:%i: rd_strnchrs#2 failed: "
		       "expected nothing, but got '%s'\n",
		       __FUNCTION__,__LINE__, m);
		fails++;
	}


	/* Look for something outside the buffer but with match_eol=1 */
	if (!(m = rd_strnchrs(str2, 3, "4", 1)) || m != str2+3) {
		printf("%s:%i: rd_strnchrs#2 failed: "
		       "expected eob, but got '%s'\n",
		       __FUNCTION__,__LINE__, m);
		fails++;
	}

	/*
	 * Test rd_strn*spn()
	 */
	str1 = "1234abcde";
	if ((i = rd_strnspn(str1, strlen(str1), "1234")) != 4) {
		printf("%s:%i: rd_strnspn#1 failed: "
		       "expected %i, got %i\n",
		       __FUNCTION__,__LINE__, 4, i);
		fails++;
	}

	if ((i = rd_strncspn(str1, strlen(str1), "1234")) != 0) {
		printf("%s:%i: rd_strncspn#1 failed: "
		       "expected %i, got %i\n",
		       __FUNCTION__,__LINE__, 0, i);
		fails++;
	}

	if ((i = rd_strncspn(str1, strlen(str1), "abcd")) != 4) {
		printf("%s:%i: rd_strncspn#2 failed: "
		       "expected %i, got %i\n",
		       __FUNCTION__,__LINE__, 4, i);
		fails++;
	}

	if ((i = rd_strncspn(str1, strlen(str1)-2, "e")) != 7) {
		printf("%s:%i: rd_strncspn#3 failed: "
		       "expected %i, got %i\n",
		       __FUNCTION__,__LINE__, 7, i);
		fails++;
	}




	/*
	 * Test rd_tsprintf() cyclicar.
	 */
	str1 = rd_tsprintf("Telma %s!", "rocks");
	str2 = rd_tsprintf("%s too!", "Magda");
	if (strcmp(str1, "Telma rocks!")) {
		printf("%s:%i: rd_tsprintf#1 failed: got '%s'\n",
		       __FUNCTION__,__LINE__, str1);
		fails++;
	}
	if (strcmp(str2, "Magda too!")) {
		printf("%s:%i: rd_tsprintf#2 failed: got '%s'\n",
		       __FUNCTION__,__LINE__, str2);
		fails++;
	}

	/*
	 * rd_strdiffpos
	 */
	if ((i = rd_strdiffpos("Magnus", "Mike")) != 1) {
		printf("%s:%i: rd_strdiffpos#1 failed: expected %i, got %i\n",
		       __FUNCTION__,__LINE__, 1, i);
		fails++;
	}

	if ((i = rd_strdiffpos("Magnus", "Magnus")) != -1) {
		printf("%s:%i: rd_strdiffpos#1 failed: expected %i, got %i\n",
		       __FUNCTION__,__LINE__, -1, i);
		fails++;
	}

	if ((i = rd_strndiffpos("Magnus", 4, "Magnolia", 4)) != -1) {
		printf("%s:%i: rd_strdiffpos#1 failed: expected %i, got %i\n",
		       __FUNCTION__,__LINE__, -1, i);
		fails++;
	}

	if ((i = rd_strndiffpos("Magnus", 6, "Magnolia", 4)) != 4) {
		printf("%s:%i: rd_strdiffpos#1 failed: expected %i, got %i\n",
		       __FUNCTION__,__LINE__, 4, i);
		fails++;
	}



	return fails;
}




int main (int argc, char **argv) {
	int fails = 0;

	fails += test_string();

	return fails ? 1 : 0;
}
