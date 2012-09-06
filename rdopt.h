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
					    char **argv,		\
					    int argc, int *argip,	\
					    void *opaque)

typedef struct rd_opt_s {
	int             ro_type;     /* Argument type (and attributes) */
#define RD_OPT_NIL    0x0            /* No argument. */
#define RD_OPT_END    0x1            /* Defines the end of the option list */
#define RD_OPT_REQ    0x2            /* Required option */

	/*
	 * RD_OPT_MUT1..4 - Mutual exclusivity groups
	 *
	 * Any options marked with the same RD_OPT_MUTn group will only
	 * allow one of them to be seen in the argument list.
	 *
	 * This works as expected with RD_OPT_REQ: it will only allow one option
	 * in a mut group to be set and it will not complain of missing .._REQ
	 * arguments if they are in a satisfied mut group.
	 */
#define RD_OPT_MUT1   0x10            /* Mutual exclusivity group 1 */
#define RD_OPT_MUT2   0x20            /* Mutual exclusivity group 2 */
#define RD_OPT_MUT3   0x40            /* Mutual exclusivity group 3 */
#define RD_OPT_MUT4   0x80            /* Mutual exclusivity group 4 */

	/* For rdopt internal use */
#define RD_OPT_MUT_MASK 0xf0
#define RD_OPT_MUT_GRP(B) (((B)&RD_OPT_MUT_MASK) >> 4)
#define RD_OPT_MUT_NUM 4

	/* The following are for rd_opt_parse_assign() use. */
#define RD_OPT_STR    0x1000         /* String argument.  opaque: char ** */
#define RD_OPT_INT    0x2000         /* Integer argument. opaque: int * */
#define RD_OPT_PATH   0x4000         /* Argument is a path that must exist.
				      * opaque: char ** */
#define RD_OPT_BOOL   0x8000         /* Non-argument option. opaque: int * */
	char            ro_short;    /* Short option, e.g., -s */
	const char     *ro_long;     /* Long option, e.g., --server */
	int             ro_argcnt;   /* Number of expected args: 0..N */
	void           *ro_opaque;   /* Parse function opaque pointer */
	const char     *ro_help;     /* Optional help text */
	const char     *ro_confname; /* Configuration token name, e.g. server*/
	rd_opt_parse_f(*ro_parse);   /* Parse function */
} rd_opt_t;



/**
 * Parse the provided argument list and map it to the options defined
 * in 'ros'.
 *
 * Returns NULL on success or an error string on failure.
 *
 * If "--help" is encountered as an option the function returns the
 * literal string "(help)" without further processing, the application
 * should usually run rd_opt_usage() and exit in this case.
 *
 * Also see: rd_opt_get() for a common wrapper.
 *
 * NOTE: Currently treats ro_argcnt as a boolean, i.e., no support
 *       for more than 1 argument per option.
 */

const char *rd_opt_parse (const rd_opt_t *ros,
			  int argc, char **argv, int *argip);


/**
 * Convenience wrapper for rd_opt_parse() and rd_opt_usage(), that on
 * failure of rd_opt_parse() prints the error message as well as the usage.
 *
 * Returns 1 on success or 0 on error.
 *
 * Example (in your main()):
 *   if (!rd_opt_get(opts, argc, argv, &nextargi, "<input-file>"))
 *       exit(1);
 *
 */
int rd_opt_get (const rd_opt_t *ros,
		int argc, char **argv, int *argip,
		const char *extra_args);


/**
 * Prints program command line option usage.
 * 'extra_args' is an optional string of non-option arguments that is appended
 * to the syntax description output.
 */
void rd_opt_usage (const rd_opt_t *ros, FILE *fp,
		   const char *argv0, const char *extra_args);


/**
 * Standard parser that assigns the value to the specified pointer
 * in the ro_opaque field according to the ro_type.
 *
 * This is the default parser if 'ro_parse' is not set.
 */
rd_opt_parse_f(rd_opt_parse_assign);

