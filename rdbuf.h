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

/* NOTE: rdbuf is work in progress. */

#pragma once

#include <stdarg.h>
#include "rdqueue.h"

typedef struct rd_buf_s {
	TAILQ_ENTRY(rd_buf_s) rb_link;
	uint32_t rb_len;       /* current length */
	uint32_t rb_olen;      /* original allocated length */
	int      rb_flags;
#define RD_BUF_F_OWNER    0x1
	char    *rb_data;      /* Read pointer */
	char    *rb_orig;      /* Beginning of buffer */
} rd_buf_t;


#define RD_BUF_READ_SIZE 256

typedef enum {
	RD_BUF_ERROR = 0,
	RD_BUF_WANT_MORE,
	RD_BUF_DONE,
} rd_buf_status_t;


/**
 * Decoder semantics:
 *  rbh is always non-NULL.
 *  rb != NULL && rb->rb_len > 0: new data received
 *  rb != NULL && rb->rb_len == 0: recv returned 0
 *  rb == NULL: recv returned -1
 *
 * The decoder must return with one of the rd_buf_status_t codes.
 */
 
#define rd_buf_decoder(f) \
 rd_buf_status_t (f) (rd_bufh_t *rbh, rd_buf_t *rb, void *opaque)

TAILQ_HEAD(rd_buf_tq_head, rd_buf_s);

typedef struct rd_bufh_s {
	TAILQ_ENTRY(rd_buf_s)   rbh_link;
	struct rd_buf_tq_head   rbh_bufs;
	uint32_t   rbh_len;
	int        rbh_flags;
#define RD_BUFH_F_FREE   0x1  /* Free rd_buf_t on destroy */
	int        rbh_read_size;
	void      *rbh_opaque;
} rd_bufh_t;

typedef struct rd_bufq_s {
	TAILQ_HEAD(, rd_bufh_s) rbq_bufhs;
	int        rbq_cnt;
	uint64_t   rbq_len;
} rd_bufq_t;



static inline uint32_t rd_bufh_len (const rd_bufh_t *rbh) {
	return rbh->rbh_len;
}

void rd_bufh_destroy (rd_bufh_t *rbh);
rd_bufh_t *rd_bufh_new (rd_bufh_t *rbh, int read_size);
rd_buf_t *rd_bufh_recv (rd_bufh_t *rbh, int s, uint32_t len);
rd_buf_t *rd_bufh_prepend (rd_bufh_t *rbh, void *data, uint32_t len, int flags);
rd_buf_t *rd_bufh_append (rd_bufh_t *rbh, void *data, uint32_t len, int flags);

rd_buf_t *rd_bufh_vsprintf (rd_bufh_t *rbh, const char *format, va_list ap);
rd_buf_t *rd_bufh_sprintf (rd_bufh_t *rbh, const char *format, ...);
void      rd_bufh_move (rd_bufh_t *dst, rd_bufh_t *src);

rd_buf_t *rd_buf_vsprintf (const char *format, va_list ap);
rd_buf_t *rd_buf_sprintf  (const char *format, ...);

void rd_bufh_buf_insert (rd_bufh_t *rbh, rd_buf_t *after,
			 rd_buf_t *rb);

/**
 * The buffer writer callback writes serialized data to the destination,
 * whatever it may be.
 * It is to be called by the serializer to write out its encoded chunks.
 */
#define rd_bufh_writer_f(f)						\
	ssize_t (f) (const rd_bufh_t *rbh, void *data, size_t len,	 \
		     void *writer_opaque)

/**
 * The buffer serializer callback encodes the buffer into a serialized form.
 */
#define rd_bufh_serializer_f(f)					      \
	ssize_t (f) (const rd_bufh_t *rbh, rd_bufh_writer_f(*writer), \
		     size_t of, const rd_buf_t *rb,		      \
		     void *serializer_opaque, void *writer_opaque)

		
ssize_t rd_bufh_serialize (const rd_bufh_t *rbh,
			   rd_bufh_serializer_f(*serializer),
			   rd_bufh_writer_f(*writer),
			   void *serializer_opaque,
			   void *writer_opaque);


/**
 * Contiguous destination memory writer,
 * i.e. memcpy() on preallocated memory.
 */
rd_bufh_writer_f(rd_bufh_write_mem);

/**
 * File descriptor writer.
 */
rd_bufh_writer_f(rd_bufh_write_fd);

/**
 * Standard binary serializer that simply copies the data without alteration.
 */
rd_bufh_serializer_f(rd_bufh_serialize_binary);


/**
 * Binary-serializes and writes the buffer to contiguous memory 'dst'.
 * i.e.: memcpy().
 */
#define rd_bufh_copyout(rbh,dst) \
	rd_bufh_serialize(rbh,rd_bufh_serialize_binary,rd_bufh_write_mem,\
			  NULL,dst)

/**
 * Binary-serializes and writes the buffer to opened for writing 
 * file descriptor 'fd'.
 * i.e: write(fd, ..)
 */
static ssize_t rd_bufh_writeout_fd (const rd_bufh_t *rbh, int fd) RD_UNUSED;
static ssize_t rd_bufh_writeout_fd (const rd_bufh_t *rbh, int fd) {
	return rd_bufh_serialize(rbh,
				 rd_bufh_serialize_binary,
				 rd_bufh_write_fd,
				 NULL, &fd);
}

void rd_bufh_dump (const char *indent, const rd_bufh_t *rbh);
