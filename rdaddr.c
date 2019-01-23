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
#include "rdaddr.h"
#include "rdstring.h"
#include "rdbits.h"
#include "rdrand.h"

const char *rd_sockaddr2str (const void *addr, int flags) {
	const rd_sockaddr_inx_t *a = (const rd_sockaddr_inx_t *)addr;
	static __thread char ret[32][INET6_ADDRSTRLEN + 16];
	static __thread int  reti = 0;
	char portstr[64];
	int of = 0;
	int niflags = NI_NUMERICSERV;

	reti = (reti + 1) % 32;
	
	switch (a->sinx_family)
	{
	case AF_INET:
	case AF_INET6:
		if (BIT_TEST(flags, RD_SOCKADDR2STR_F_FAMILY))
			of += sprintf(&ret[reti][of], "ipv%i#",
				      a->sinx_family == AF_INET ? 4 : 6);

		if (BIT_TEST(flags, RD_SOCKADDR2STR_F_PORT) &&
		    a->sinx_family == AF_INET6)
			ret[reti][of++] = '[';

		if (!BIT_TEST(flags, RD_SOCKADDR2STR_F_RESOLVE))
			niflags |= NI_NUMERICHOST;

		if (getnameinfo((const struct sockaddr *)a,
				RD_SOCKADDR_INX_LEN(a),
				ret[reti]+of, sizeof(ret[reti])-of,
				BIT_TEST(flags, RD_SOCKADDR2STR_F_PORT) ?
				portstr : NULL,
				BIT_TEST(flags, RD_SOCKADDR2STR_F_PORT) ?
				sizeof(portstr) : 0,
				niflags))
			break;

		
		if (BIT_TEST(flags, RD_SOCKADDR2STR_F_PORT))
			rd_snprintf_cat(ret[reti], sizeof(ret[reti]),
					"%s:%s",
					a->sinx_family == AF_INET6 ? "]" : "",
					portstr);
	
		return ret[reti];
	}
	

	/* Error-case */
	snprintf(ret[reti], sizeof(ret[reti]), "<unsupported:%s>",
		 rd_family2str(a->sinx_family));
	
	return ret[reti];
}


const char *rd_addrinfo_prepare (const char *nodesvc,
				 char **node, char **svc) {
	static __thread char snode[256];
	static __thread char ssvc[64];
	const char *t;
	const char *svct = NULL;
	int nodelen = 0;

	*snode = '\0';
	*ssvc = '\0';

	if (*nodesvc == '[') {
		/* "[host]".. (enveloped node name) */
		if  (!(t = strchr(nodesvc, ']')))
			return "Missing close-']'";
		nodesvc++;
		nodelen = (int)(t-nodesvc);
		svct = t+1;

	} else if (*nodesvc == ':' && *(nodesvc+1) != ':') {
		/* ":"..  (port only) */
		nodelen = 0;
		svct = nodesvc;
	}
		
	if ((svct = strrchr(svct ? : nodesvc, ':')) && (*(svct-1) != ':') &&
	    *(++svct)) {
		/* Optional ":service" definition. */
		if (strlen(svct) >= sizeof(ssvc))
			return "Service name too long";
		strcpy(ssvc, svct);
		if (!nodelen)
			nodelen = (int)(svct - nodesvc)-1;

	} else if (!nodelen)
		nodelen = strlen(nodesvc);

	if (nodelen) {
		strncpy(snode, nodesvc, nodelen);
		snode[nodelen] = '\0';
	}

	*node = snode;
	*svc = ssvc;

	return NULL;
}



rd_sockaddr_list_t *rd_getaddrinfo (const char *nodesvc, const char *defsvc,
				    int flags, int family,
				    int socktype, int protocol,
				    const char **errstr) {
	struct addrinfo hints = { .ai_family = family,
				  .ai_socktype = socktype,
				  .ai_protocol = protocol,
				  .ai_flags = flags };
	struct addrinfo *ais, *ai;
	char *node, *svc;
	int r;
	int cnt = 0;
	rd_sockaddr_list_t *rsal;

	if ((*errstr = rd_addrinfo_prepare(nodesvc, &node, &svc))) {
		errno = EINVAL;
		return NULL;
	}

	if (*svc)
		defsvc = svc;
		
	if ((r = getaddrinfo(node, defsvc, &hints, &ais))) {
		*errstr = gai_strerror(r);
		errno = EFAULT;
		return NULL;
	}
	
	/* Count number of addresses */
	for (ai = ais ; ai != NULL ; ai = ai->ai_next)
		cnt++;

	if (cnt == 0) {
		/* unlikely? */
		freeaddrinfo(ais);
		errno = ENOENT;
		*errstr = "No addresses";
		return NULL;
	}


	rsal = calloc(1, sizeof(*rsal) + (sizeof(*rsal->rsal_addr) * cnt));

	for (ai = ais ; ai != NULL ; ai = ai->ai_next)
		memcpy(&rsal->rsal_addr[rsal->rsal_cnt++],
		       ai->ai_addr, ai->ai_addrlen);

	freeaddrinfo(ais);

	/* Shuffle address list for proper round-robin */
	if (!BIT_TEST(flags, RD_AI_NOSHUFFLE))
		rd_array_shuffle(rsal->rsal_addr, rsal->rsal_cnt,
				 sizeof(*rsal->rsal_addr));

	return rsal;
}



void rd_sockaddr_list_destroy (rd_sockaddr_list_t *rsal) {
	free(rsal);
}

