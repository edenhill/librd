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
#include "rdbuf.h"
#include "rdbits.h"
#include "rdlog.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <stdarg.h>

/** FIXME: THREAD SAFE */
/** FIXME: Use (s)size_t for >32 bit sizes? */

static inline void rd_bufh_update_len (rd_bufh_t *rbh, rd_buf_t *rb,
				       int32_t len) {
	rb->rb_len += len;
	rbh->rbh_len += len;
}

/**
 * Frees a buffer that must have been previously unlinked from the bufh.
 */
void rd_buf_destroy0 (rd_buf_t *rb) {
	if (BIT_TEST(rb->rb_flags, RD_BUF_F_OWNER))
		free(rb->rb_orig);
	free(rb);
}

/**
 * Frees and unlinks a buffer and updates the length of the buffer head.
 */
void rd_buf_destroy (rd_bufh_t *rbh, rd_buf_t *rb) {
	TAILQ_REMOVE(&rbh->rbh_bufs, rb, rb_link);
	rd_bufh_update_len(rbh, rb, -rb->rb_len);
	rd_buf_destroy0(rb);
}


void rd_bufh_destroy (rd_bufh_t *rbh) {
	rd_buf_t *rb, *nrb;

	TAILQ_FOREACH_SAFE(rb, &rbh->rbh_bufs, rb_link, nrb)
		rd_buf_destroy(rbh, rb);

	assert(rbh->rbh_len == 0);

	if (rbh->rbh_flags & RD_BUFH_F_FREE)
		free(rbh);
}


rd_bufh_t *rd_bufh_new (rd_bufh_t *rbh, int read_size) {

	if (!rbh) {
		rbh = calloc(1, sizeof(*rbh));
		rbh->rbh_flags = RD_BUFH_F_FREE;
	} else
		memset(rbh, 0, sizeof(*rbh));

	rbh->rbh_read_size = read_size ? : RD_BUF_READ_SIZE;

	TAILQ_INIT(&rbh->rbh_bufs);

	return rbh;
}


#if 0
rd_buf_status_t rd_bufh_recvx (int s, rd_bufh_t *rbh, uint32_t len,
			       rd_buf_decoder_f(*decoder), void *opaque) {
	int r;
	rd_buf_t *rb;
	rd_buf_status_t st;

	do {
		/* FIXME: If 'len' is larger than what fits in a single
		 * buffer (i.e., malloc failure), then add support for
		 * chained buf reads here. */
		rb = rd_buf_add(rbh, rbh->rbh_len);
		r = recv(s, rb->rb_data, len, 0);
		
		st = decoder(rbh, r == -1 ? NULL : rb, opaque);
	} while (st == RD_BUF_WANT_MORE);

	return st;
}
#endif


rd_buf_t *rd_buf_new (void *data, uint32_t len, int flags) {
	rd_buf_t *rb;

	if (!BIT_TEST(flags, RD_BUF_F_OWNER)) {
		/* We are to make a copy of the data, allocate size for
		 * the data after the buf structure so it ends up in just one
		 * allocation. */

		/* Make sure the allocation is large enough to avoid senseless
		 * overhead by the buf struct itself. */
		if (len < sizeof(*rb))
			len = sizeof(*rb);

		rb = malloc(sizeof(*rb) + len);
		memset(rb, 0, sizeof(*rb));
		rb->rb_orig = (char *)(rb+1);
	} else {
		/* We will own the `data' buffer (pre-allocated by caller). */
		rb = calloc(1, sizeof(*rb));
		rb->rb_orig = data;
	}


	rb->rb_data  = rb->rb_orig;
	rb->rb_olen  = len;
	rb->rb_flags = flags;

	return rb;
}

static inline int rd_buf_remaining (const rd_buf_t *rb) RD_UNUSED;
static inline int rd_buf_remaining (const rd_buf_t *rb) {
	return rb->rb_olen - rb->rb_len;
}

void rd_buf_append_data (rd_bufh_t *rbh, rd_buf_t *rb,
			 const void *data, uint32_t len) {
	assert(rd_buf_remaining(rb) >= len);
	memcpy(rb->rb_orig + rb->rb_len, data, len);
	rd_bufh_update_len(rbh, rb, len);
}


void rd_bufh_buf_insert (rd_bufh_t *rbh, rd_buf_t *after,
			 rd_buf_t *rb) {

	if (after)
		TAILQ_INSERT_AFTER(&rbh->rbh_bufs, after, rb, rb_link);
	else
		TAILQ_INSERT_HEAD(&rbh->rbh_bufs, rb, rb_link);

	rbh->rbh_len += rb->rb_len;
}



rd_buf_t *rd_bufh_get_buf (rd_bufh_t *rbh,
			   void *data, uint32_t len, int flags) {
	rd_buf_t *tail, *rb;

	tail = TAILQ_LAST(&rbh->rbh_bufs, rd_buf_tq_head);

	if (!BIT_TEST(flags, RD_BUF_F_OWNER) && 
	    tail && rd_buf_remaining(tail) >= len) {
		rb = tail;
	} else {
		rb = rd_buf_new(data, len, flags);
		rd_bufh_buf_insert(rbh, tail, rb);
	}

	if (data)
		rd_buf_append_data(rbh, rb, data, len);

	return rb;
}


/**
 * Read 'len' messages and return the first buffer that points to that
 * payload.
 */
rd_buf_t *rd_bufh_recv (rd_bufh_t *rbh, int s, uint32_t len) {
	int r;
	rd_buf_t *rb;

	rb = rd_bufh_get_buf(rbh, NULL, len, 0);

	r = recv(s, rb->rb_data+rb->rb_len, len, 0);
	if (r == -1)
		return NULL;

	rd_bufh_update_len(rbh, rb, r);

	return rb;
}



rd_buf_t *rd_bufh_append (rd_bufh_t *rbh, void *data, uint32_t len,
			  int flags) {
	rd_buf_t *rb;

	rb = rd_bufh_get_buf(rbh, data, len, flags);

	return rb;
}

rd_buf_t *rd_bufh_prepend (rd_bufh_t *rbh, void *data, uint32_t len,
			   int flags) {
	rd_buf_t *rb;

	rb = rd_buf_new(data, len, flags);
	rd_bufh_buf_insert(rbh, NULL, rb);

	return rb;
}



rd_buf_t *rd_bufh_vsprintf (rd_bufh_t *rbh, const char *format, va_list ap) {
	int totlen;
	rd_buf_t *rb;
	va_list ap2;

	va_copy(ap2, ap);

	/* Calculate needed length */
	totlen = vsnprintf(NULL, 0, format, ap);

	/* Allocate buffer. */
	rb = rd_bufh_get_buf(rbh, NULL, totlen+1, 0/*FIXME: .._F_SEQ*/);

	/* Format the string to the buffer. */
	vsnprintf(rb->rb_orig+rb->rb_len, totlen+1, format, ap2);

	/* NOTE: Without trailing null */
	rd_bufh_update_len(rbh, rb, totlen);

	return rb;
}

rd_buf_t *rd_bufh_sprintf (rd_bufh_t *rbh, const char *format, ...) {
	rd_buf_t *rb;
	va_list ap;

	va_start(ap, format);
	rb = rd_bufh_vsprintf(rbh, format, ap);
	va_end(ap);

	return rb;
}




rd_buf_t *rd_buf_vsprintf (const char *format, va_list ap) {
	int totlen;
	rd_buf_t *rb;
	va_list ap2;

	va_copy(ap2, ap);

	/* Calculate needed length */
	totlen = vsnprintf(NULL, 0, format, ap);

	/* Allocate buffer. */
	rb = rd_buf_new(NULL, totlen+1, 0);

	/* Format the string to the buffer. */
	vsnprintf(rb->rb_orig+rb->rb_len, totlen+1, format, ap2);
	
	/* NOTE: Without trailing null */
	rb->rb_len = totlen;

	return rb;
}

rd_buf_t *rd_buf_sprintf (const char *format, ...) {
	rd_buf_t *rb;
	va_list ap;

	va_start(ap, format);
	rb = rd_buf_vsprintf(format, ap);
	va_end(ap);

	return rb;
}


/*
 * Moves all buffers from 'src' to tail of 'dst'.
 * 'src' will become empty.
 */
void rd_bufh_move (rd_bufh_t *dst, rd_bufh_t *src) {
	rd_buf_t *rb, *next, *tail;
	
	tail = TAILQ_LAST(&dst->rbh_bufs, rd_buf_tq_head);
	next = TAILQ_FIRST(&src->rbh_bufs);
	while (next) {
		rb = next;
		next = TAILQ_NEXT(next, rb_link);

		rd_bufh_buf_insert(dst, tail, rb);
		tail = rb;
	}

	/* Reset source bufh */
	TAILQ_INIT(&src->rbh_bufs);
	src->rbh_len = 0;
}



/**
 * Optimized serializer+writer combo for binary output to continuous memory.
 */
static size_t rd_bufh_serialize_binary_mem0 (const rd_bufh_t *rbh, void *dst) {
	const rd_buf_t *rb;
	size_t len = 0;

	TAILQ_FOREACH(rb, &rbh->rbh_bufs, rb_link) {
		memcpy(((char *)dst)+len, rb->rb_orig, rb->rb_len);
		len += rb->rb_len;
	}
	return len;
}


rd_bufh_writer_f(rd_bufh_write_mem) {
	char *dst = writer_opaque;
	memcpy(dst, data, len);
	dst += len;
	return len;
}

rd_bufh_writer_f(rd_bufh_write_fd) {
	int fd = *(int *)writer_opaque;
	return write(fd, data, len);
}

rd_bufh_serializer_f(rd_bufh_serialize_binary) {
	return writer(rbh, rb->rb_orig, rb->rb_len, writer_opaque);
}

ssize_t rd_bufh_serialize (const rd_bufh_t *rbh,
			  rd_bufh_serializer_f(*serializer),
			  rd_bufh_writer_f(*writer),
			  void *serializer_opaque,
			  void *writer_opaque) {
	const rd_buf_t *rb;
	size_t len;

	/* Use the optimized serializer for binary output, since its common. */
	if (serializer == rd_bufh_serialize_binary &&
	    writer == rd_bufh_write_mem)
		return rd_bufh_serialize_binary_mem0(rbh, writer_opaque/*dst*/);

	len = 0;
	TAILQ_FOREACH(rb, &rbh->rbh_bufs, rb_link) {
		ssize_t r;

		if (rb->rb_len == 0)
			continue;

		if ((r = serializer(rbh, writer, len, rb,
				    serializer_opaque, writer_opaque)) == -1)
			return -1;
		else if (r == 0)
			break;

		len += r;
	}

	return len;
}



void rd_bufh_dump (const char *indent, const rd_bufh_t *rbh) {
	const rd_buf_t *rb;

	printf("%sbufhead %p: length %u, flags 0x%x\n",
	       indent, rbh, rbh->rbh_len, rbh->rbh_flags);
	TAILQ_FOREACH(rb, &rbh->rbh_bufs, rb_link) {
		printf("%s  buf %p: length %u/%u, read offset %u, flags 0x%x\n",
		       indent, rb, rb->rb_len, rb->rb_olen,
		       (uint32_t)(rb->rb_data - rb->rb_orig),
		       rb->rb_flags);
	}
}
