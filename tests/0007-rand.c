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
#include <rdrand.h>
#include <rdtime.h>


static int test_shuffle (void) {
	int i;
	int fails = 0;

	{
#define _ARSZ 256
		int arr1[_ARSZ];
		int arr2[_ARSZ];

		/* Initialize arrays to a known sequence. */
		for (i = 0 ; i < _ARSZ ; i++)
			arr1[i] = arr2[i] = i;

		/* Simple test to see that it shuffles at all. */
		rd_array_shuffle(arr1, _ARSZ, sizeof(*arr1));
		if (!memcmp(arr1, arr2, sizeof(arr1))) {
			printf("rd_array_shuffle test failed: "
			       "arr1 did not shuffle\n");
			fails++;
		}
	}


	/* Shuffle biasing test. */
	{
		int stats3[321+1] = {};
		int stats3map[6] = { 123, 213, 231, 321, 312, 132 };
		const int cnt3 = 500000;
		const int avg3 = cnt3 / 6;
		const int maxdev3 = avg3 / 80; /* max deviation from avg */

		while (i++ < cnt3) {
			char arr3[3] = {1, 2, 3};
			rd_array_shuffle(arr3, 3, sizeof(*arr3));
			stats3[(arr3[2] * 100) + (arr3[1] * 10) + arr3[0]]++;
		}

		for (i = 0 ; i < RD_ARRAYSIZE(stats3map) ; i++) {
			int c = stats3[stats3map[i]];
			if (c < avg3 - maxdev3 ||
			    c > avg3 + maxdev3) {
				printf("rd_array_shuffle biastest failed: "
				       "combo %i misrepresented/biased: "
				       "%i (outside +-%i from average %i)\n",
				       stats3map[i], stats3[stats3map[i]],
				       maxdev3, avg3);
				fails++;
			}
		}
	}


	return fails;
}



int main (int argc, char **argv) {
	int fails = 0;

	srand(rd_clock());
	fails += test_shuffle();

	return fails ? 1 : 0;

}
