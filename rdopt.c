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
#include "rdopt.h"
#include "rdbits.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


static const char *rd_opt_syntaxname (const rd_opt_t *ro) {
	static char ret[128][4];
	static int i = 0;
	int retof = 0;

	i = (i + 1) % 4;
	*ret[i] = '\0';

	if (ro->ro_short)
		retof += snprintf(ret[i]+retof, sizeof(ret[i])-retof,
				  "-%c%s",
				  ro->ro_short,
				  ro->ro_long ? "/" : "");

	if (ro->ro_long)
		retof += snprintf(ret[i]+retof, sizeof(ret[i])-retof,
				  "--%s", ro->ro_long);

	if (ro->ro_argcnt)
		retof += snprintf(ret[i]+retof, sizeof(ret[i])-retof,
				  " <...>");

	return ret[i];
}


const char *rd_opt_parse (const rd_opt_t *ros,
			  int argc, char **argv, int *argip) {
	static char ret[1024];
	int retof;
	const rd_opt_t *ro = ros;
	rd_bitvec_t opts_found;
	int missed_reqs = 0;
	int argi;
	int roi;
	const rd_opt_t *mutro[RD_OPT_MUT_NUM+1] = {};

	rd_bitvec_init(&opts_found, RD_BITVEC_STATIC, 512);

	/* Skip argv0 */
	for (argi = 1; argi < argc ; argi++) {
		const char *longopt = NULL;
		const char *val = NULL;
		int newi = argi;
		const char *t;
		rd_opt_parse_f(*parser);

		if (!*argv[argi])
			continue; /* Empty argument */

		if (*argv[argi] != '-')
			break; /* First non-option, we're done. */

		if (!strcmp(argv[argi], "--help"))
			return "(help)";

		if (argv[argi][1] == '-')
			longopt = &argv[argi][2];
		else if (argv[argi][2] != '\0')
			val = &argv[argi][2];

		/* Match option */
		for (ro = ros, roi = 0 ;
		     ro->ro_type != RD_OPT_END ; ro++, roi++) {
			if (!longopt && ro->ro_short) {
				if (ro->ro_short != argv[argi][1])
					continue;

				if (ro->ro_argcnt && !val && argi + 1 < argc)
					val = argv[++newi];

			} else if (longopt && ro->ro_long) {
				if (strcmp(longopt, ro->ro_long))
					continue;

				if ((t = strchr(longopt, '=')))
					val = t+1;
				else if (ro->ro_argcnt) {
					if (argi + 1 < argc)
						val = argv[++newi];
				}
			} else
				continue;


			if (!ro->ro_argcnt && val) {
				snprintf(ret, sizeof(ret),
					 "Option %s does"
					 "not expect any "
					 "arguments",
					 rd_opt_syntaxname(ro));
				rd_bitvec_free(&opts_found);
				return ret;
			} else if (ro->ro_argcnt && !val) {
				snprintf(ret, sizeof(ret),
					 "Missing arguments to option %s",
					 rd_opt_syntaxname(ro));
				rd_bitvec_free(&opts_found);
				return ret;
			}

			/* Check mutual exclusitivity. */
			if (BIT_TEST(ro->ro_type, RD_OPT_MUT_MASK)) {
				int mg = RD_OPT_MUT_GRP(ro->ro_type);
				if (mutro[mg]) {
					snprintf(ret, sizeof(ret),
						 "Option %s is mutually "
						 "exclusive with already set "
						 "option %s",
						 rd_opt_syntaxname(ro),
						 rd_opt_syntaxname(mutro[mg]));
					rd_bitvec_free(&opts_found);
					return ret;
				}
				
				mutro[mg] = ro;
			}

			/* Use default assignment parser if no parser
			 * is set but a specific pointer is. */
			if (!(parser = ro->ro_parse) && ro->ro_opaque)
				parser = rd_opt_parse_assign;

			if (parser) {
				if ((t = parser(ro, argv, argc,
						&newi, ro->ro_opaque))) {
					rd_bitvec_free(&opts_found);
					return t;
				}
			}


			rd_bitvec_set(&opts_found, roi);

			argi = newi;
			break;
		}


		if (ro->ro_type == RD_OPT_END) {
			snprintf(ret, sizeof(ret),
				 "Unknown option: %s", argv[argi]);
			rd_bitvec_free(&opts_found);
			return ret;
		}
	}
	
	retof = snprintf(ret, sizeof(ret), "Required option(s) missing:");

	/* Check that all required options have been passed. */
	for (ro = ros, roi = 0 ;
	     !BIT_TEST(ro->ro_type, RD_OPT_END) ;
	     ro++, roi++) {
		if (!BIT_TEST(ro->ro_type, RD_OPT_REQ))
			continue;

		if (rd_bitvec_test(&opts_found, roi))
			continue;

		/* Required values in mutual exclusitivity groups only
		 * require one of the mutual excl. options to have been set. */
		if (BIT_TEST(ro->ro_type, RD_OPT_MUT_MASK) &&
		    mutro[RD_OPT_MUT_GRP(ro->ro_type)])
			continue;

		/* Required option not passed, construct error string. */
		retof += snprintf(ret+retof, sizeof(ret)-retof, "%s ",
				  missed_reqs ? "," : "");

		missed_reqs++;

		if (ro->ro_short)
			retof += snprintf(ret+retof, sizeof(ret)-retof,
					  "-%c%s",
					  ro->ro_short,
					  ro->ro_long ? "/" : "");

		if (ro->ro_long)
			retof += snprintf(ret+retof, sizeof(ret)-retof,
					  "%s", ro->ro_long);

		if (ro->ro_argcnt)
			retof += snprintf(ret+retof, sizeof(ret)-retof,
					  " <...>");
	}

	rd_bitvec_free(&opts_found);

	if (missed_reqs)
		return ret;

	if (argip)
		*argip = argi;
	
	return NULL;
}


rd_opt_parse_f(rd_opt_parse_assign) {
	static char ret[512];

	if (ro->ro_argcnt == 0) {
		if (opaque && ro->ro_type & (RD_OPT_BOOL|RD_OPT_INT))
			*(int *)opaque = 1;

		return NULL;
	}

	/* Assuming just one argument. */
	if (ro->ro_type & (RD_OPT_INT|RD_OPT_BOOL)) {
		char *end;
		int v = strtol(argv[*argip], &end, 0);

		if (!v && end == argv[*argip]) {
			snprintf(ret, sizeof(ret),
				 "Option %s requires integer argument",
				 rd_opt_syntaxname(ro));
			return ret;
		}

		if (ro->ro_type & RD_OPT_BOOL)
			v = !!v;

		if (opaque)
			*(int *)opaque = v;

		return NULL;

	} else if (ro->ro_type & RD_OPT_PATH) {
		struct stat st;

		if (stat(argv[*argip], &st) == -1) {
			snprintf(ret, sizeof(ret),
				 "Option %s path %s: %s",
				 rd_opt_syntaxname(ro), argv[*argip],
				 strerror(errno));
			return ret;
		}

		if (opaque)
			*(const char **)opaque = argv[*argip];

		return NULL;

	} else if (ro->ro_type & RD_OPT_STR) {
		/* Else treat is as a string */
		if (!*argv[*argip]) {
			snprintf(ret, sizeof(ret),
				 "Option %s requires an argument",
				 rd_opt_syntaxname(ro));
			return ret;
		}
	
		if (opaque)
			*(const char **)opaque = argv[*argip];

		return NULL;
	}

	return NULL;
}



void rd_opt_usage (const rd_opt_t *ros, FILE *fp,
		   const char *argv0, const char *extra_args) {
	const rd_opt_t *ro;
	int confnamecnt = 0;
	int mutcnt[RD_OPT_MUT_NUM+1] = {};
	int mg;

	if (!ros || BIT_TEST(ros->ro_type, RD_OPT_END)) {
		/* No options */
		fprintf(fp, "Usage: %s%s%s\n\n",
			argv0, extra_args ? " " : "",
			extra_args ? : "");
		return;
	}

	fprintf(fp, 
		"Usage: %s [options]%s%s\n"
		"\n"
		"Options:\n",
		argv0, extra_args ? " " : "",
		extra_args ? : "");

	for (ro = ros ; !BIT_TEST(ro->ro_type, RD_OPT_END) ; ro++) {
		char buf[40];
		int of = 0;

		char mgstr[2];

		if (ro->ro_short && ro->ro_long)
			of = snprintf(buf, sizeof(buf), "-%c | --%s",
				      ro->ro_short, ro->ro_long);
		else if (ro->ro_short)
			of = snprintf(buf, sizeof(buf), "-%c", ro->ro_short);
		else if (ro->ro_long)
			of = snprintf(buf, sizeof(buf), "--%s", ro->ro_long);
		else
			continue;


		if (ro->ro_argcnt > 0) {
			const char *typestr = "..";

			if (ro->ro_type & RD_OPT_STR)
				typestr = "string";
			else if (ro->ro_type & RD_OPT_INT)
				typestr = "integer";
			else if (ro->ro_type & RD_OPT_PATH)
				typestr = "path";
			else if (ro->ro_type & RD_OPT_BOOL)
				typestr = "bool:1|0";

			of += snprintf(buf+of, sizeof(buf)-of,
				       " <%s>", typestr);
		}

		if ((mg = RD_OPT_MUT_GRP(ro->ro_type))) {
			mutcnt[mg]++;
			snprintf(mgstr, sizeof(mgstr), "%i", mg);
		} else
			strcpy(mgstr, " ");

		fprintf(fp, " %s%s  %-28s  ", 
			ro->ro_type & RD_OPT_REQ ? "*" : " ", mgstr, buf);


		if (ro->ro_confname) {
			fprintf(fp, "{%s} ", ro->ro_confname);
			confnamecnt++;
		}

		if (ro->ro_help)
			fprintf(fp, "%s", ro->ro_help);

		fprintf(fp, "\n");
	}

	
	
	fprintf(fp,
		"\n"
		" *     = Required option\n");

	if (confnamecnt)
		fprintf(fp, " {..}  = Configuration file token\n");

	for (mg = 1 ; mg < RD_OPT_MUT_NUM+1 ; mg++)
		if (mutcnt[mg])
			fprintf(fp, " %i     = Mutual exclusive group #%i\n",
				mg, mg);

	fprintf(fp, "\n");

}


int rd_opt_get (const rd_opt_t *ros,
		int argc, char **argv, int *argip,
		const char *extra_args) {
	
	const char *errstr;

	if ((errstr = rd_opt_parse(ros, argc, argv, argip))) {
		if (strcmp(errstr, "(help)"))
			fprintf(stderr, "%s: %s\n\n", argv[0], errstr);
		rd_opt_usage(ros, stderr, argv[0], extra_args);
		return 0;
	}

	return 1;
}
