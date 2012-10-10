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


#include <inttypes.h>
#include <stdlib.h>

/**
 * Various encoding/decoding schemes.
 */



/**
 *
 * Varint decoding & encoding
 *
 * https://developers.google.com/protocol-buffers/docs/encoding#varints
 *
 */

#define RD_VARINT_DECODE_ERR(vlen)       ((vlen) <= 0)
#define RD_VARINT_DECODE_OVERFLOW(vlen)  ((vlen) <= -9)
#define RD_VARINT_DECODE_UNDERFLOW(vlen) (-(vlen))

/**
 * Decode a variant at 'buf' (buffer is of size 'len') and return
 * it as a uint64. The read length is stored in '*vlenp'.
 * If '*vlenp' is < -9 the varint would overflow.
 * If '*vlenp' is <= 0 > -9 the varint could not be decoded due to
 * buffer shortage.
 */
uint64_t rd_varint_decode_u64 (const void *buf, size_t size, int *vlenp);

/**
 * Same as rd_varint_decode_u64 but returns a signed int64 instead.
 */
int64_t rd_varint_decode_s64 (const void *buf, size_t size, int *vlenp);


/**
 * Encodes a uint64 as a varint in destination buffer 'dest'.
 * Returns the number of written bytes on success or -1 on buffer shortage.
 */
int rd_varint_encode_u64 (uint64_t uval, void *dest, size_t size);

/**
 * Same as rd_varint_encode_u64() but for signed int64 instead.
 */
int rd_varint_encode_s64 (int64_t val, void *dest, size_t size);
