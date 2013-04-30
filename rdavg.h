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


/**
 * NOTE: THIS IS WORK IN PROGRESS
 **/

#include "rdsysqueue.h"

typedef enum {
	RD_AVG_RATE,  /* x/time-interval rate */
	RD_AVG_HIST,  /* fixed-bucket histogram */

} rd_avg_type_t;

#define RD_AVG_NO_VALUE 0xffffffffffffffff

/* Result struct */
typedef struct rd_avg_res_s {
	double   dbl;
	uint64_t sum;
	uint64_t high;
	uint64_t low;
} rd_avg_res_t;

typedef struct rd_avg_rate_s {
	uint64_t cnt;
} rd_avg_rate_t;

typedef struct rd_avg_hist_s {
	uint64_t *bucket;
} rd_avg_hist_t;

typedef struct rd_avg_period_s {
	rd_ts_t last;       /* last data point */
	rd_ts_t duration;
	rd_ts_t closed;
	int     missed;
	union {
		rd_avg_rate_t rate;
		rd_avg_hist_t hist;
	} u;
	rd_avg_res_t res;
} rd_avg_period_t;

typedef struct rd_avg_s {
	TAILQ_ENTRY(rd_avg_s) ra_link;

	rd_avg_type_t       ra_type;
	rd_avg_period_t    *ra_curr;     /* current period */
	int                 ra_periods;  /* current + previous periods */
	rd_avg_period_t    *ra_period;
	int                 ra_curri;    /* current period index */
			    
	int                 ra_duration; /* config: period duration */
			    
	rd_ts_t             ra_start;    /* start time of current period */
	rd_ts_t             ra_end;      /* future stop time of current period*/

	/* Type-specific configuration */
	union {
		struct {
			int interval;
		} rate;
		struct {
			int buckets;
			int (*val2bucket) (struct rd_avg_s *ra,
					   uint64_t val, int buckets);
		} hist;
	} ra_u;

	struct rd_avg_s  *ra_parent;
} rd_avg_t;




#define RD_AVG_CURR  -1  /* current period */
#define RD_AVG_PREV  -2  /* previous period */

rd_avg_t *rd_avg_new_rate (int periods, int duration, int interval);
rd_avg_res_t rd_avg (rd_avg_t *ra, int period);
void rd_avg_start (rd_avg_t *ra);
void rd_avg_put (rd_avg_t *ra, uint64_t val);
