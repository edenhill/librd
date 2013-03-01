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
#include "rdevent.h"


struct func_cb_ctx {
	int   argcnt;
	void *args[4];
	void *cb;
};

static rd_thread_event_f(rd_thread_event_func_cb) {
	struct func_cb_ctx *ctx = ptr;

	/* This might look silly, but it's convenient. */
	switch (ctx->argcnt)
	{
	case 0:
		((void (*)(void))
		 (ctx->cb))();
		break;

	case 1:
		((void (*)(void *))
		  (ctx->cb))(ctx->args[0]);
		break;

	case 2:
		((void (*)(void *, void *))
		 (ctx->cb))(ctx->args[0],
			    ctx->args[1]);
		break;

	case 3:
		((void (*)(void *, void *, void *))
		 (ctx->cb))(ctx->args[0],
			    ctx->args[1],
			    ctx->args[2]);
		break;

	case 4:
		((void (*)(void *, void *, void *, void *))
		 (ctx->cb))(ctx->args[0],
			    ctx->args[1],
			    ctx->args[2],
			    ctx->args[3]);
		break;

	default:
		assert(!*"rd_thread_func_call: invalid argcnt");
	}

	free(ctx);
}


void rd_thread_func_call (rd_thread_t *rdt, void *cb, int argcnt, void **args) {
	struct func_cb_ctx *ctx;

	ctx = malloc(sizeof(*ctx));
	ctx->cb = cb;

	assert(argcnt <= 4);
	for (ctx->argcnt = 0 ; ctx->argcnt < argcnt ; ctx->argcnt++)
		ctx->args[ctx->argcnt] = args[ctx->argcnt];

	rd_thread_event_add(rdt, rd_thread_event_func_cb, ctx);
}
