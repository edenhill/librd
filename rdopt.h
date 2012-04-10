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

#pragma once


#define rd_opt_parse_f(f)  const char *(f) (const struct rd_opt_s *ro,	\
					    const char *const *argv,	\
					    int argc, int *argip,	\
					    void *opaque)

typedef struct rd_opt_s {
	int             ro_type;     /* Argument type (and attributes) */
#define RD_OPT_NIL    0x0            /* No argument */
#define RD_OPT_END    0x1            /* Defines the end of the option list */
#define RD_OPT_REQ    0x2            /* Required option */
	/* The following are for rd_opt_parse_assign() use. */
#define RD_OPT_STR    0x8            /* String argument.  opaque: char ** */
#define RD_OPT_INT    0x10           /* Integer argument. opaque: int * */
#define RD_OPT_PATH   0x20           /* Argument is a path that must exist.
				      * opaque: char ** */
#define RD_OPT_BOOL   0x40           /* Non-argument option. opaque: int * */
	char            ro_short;    /* Short option, e.g., -s */
	const char     *ro_long;     /* Long option, e.g., --server */
	int             ro_argcnt;   /* Number of expected args: 0..N */
	void           *ro_opaque;   /* Parse function opaque pointer */
	const char     *ro_help;     /* Optional help text */
	const char     *ro_confname; /* Configuration token name, e.g. server*/
	rd_opt_parse_f(*ro_parse);   /* Parse function */
} rd_opt_t;

rd_opt_parse_f(rd_opt_parse_assign);

const char *rd_opt_parse (const rd_opt_t *ros,
			  int argc, const char * const *argv, int *argip);


void rd_opt_usage (const rd_opt_t *ros, FILE *fp,
		   const char *argv0, const char *extra_args);
