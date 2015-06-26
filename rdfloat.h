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

#pragma once

#include <math.h>

/**
 * rd_deq0(a,b,prec)
 * Check two doubles for equality with the specified precision.
 * Use this instead of != and == for all floats/doubles.
 * More info:
 *  http://docs.sun.com/source/806-3568/ncg_goldberg.html
 */
static inline int rd_deq0 (double a, double b, double prec) RD_UNUSED;
static inline int rd_deq0 (double a, double b, double prec) {
  return fabs(a - b) < prec;
}

/* A default 'good' double-equality precision value.
 * This rather timid epsilon value is useful for tenths, hundreths,
 * and thousands parts, but not anything more precis than that.
 * If a higher precision is needed, use deq0 and deq0 directly
 * and specify your own precision. */
#define RD_DBL_EPSILON 0.00001

/**
 * rd_deq(a,b)
 * Same as rd_deq0() above but with a predefined 'good' precision.
 */
#define rd_deq(a,b) rd_deq0(a,b,RD_DBL_EPSILON)

/**
 * rd_dne(a,b)
 * Same as rd_deq() above but with reversed logic: not-equal.
 */
#define rd_dne(a,b) (!rd_deq0(a,b,RD_DBL_EPSILON))

/**
 * rd_dz(a)
 * Checks if the double `a' is zero (or close enough).
 */
#define rd_dz(a)  rd_deq0(a,0.0,RD_DBL_EPSILON)

/**
 * rd_dnz(a)
 * Checks if the double `a' is not zero.
 */
#define rd_dnz(a) (!rd_deq0(a,0.0,RD_DBL_EPSILON))
