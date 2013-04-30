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
#include "rdavg.h"
#include "rdtime.h"

/**
 * NOTE: THIS IS WORK IN PROGRESS
 **/


rd_avg_t *rd_avg_new (rd_avg_type_t type, int periods, int duration) {
	rd_avg_t *ra;

	ra = calloc(1, sizeof(*ra));

	ra->ra_type     = type;
	ra->ra_periods  = periods;
	ra->ra_duration = duration;
	return ra;
}



rd_avg_t *rd_avg_new_rate (int periods, int duration, int interval) {
	rd_avg_t *ra;

	if (periods < 1 || periods > 10000) {
		errno = EINVAL;
		return NULL;
	}

	ra = rd_avg_new(RD_AVG_RATE, periods, duration);

	ra->ra_u.rate.interval = interval;

	ra->ra_period = calloc(periods, sizeof(*ra->ra_period));

	return ra;
}

static const rd_avg_res_t *rd_avg_calc (rd_avg_t *ra, rd_avg_period_t *p) {
	rd_avg_res_t *res = &p->res;

	if (p->duration == 0) {
		/* No data values */
	no_data:
		memset(res, 0, sizeof(*res));
		return res;
	}

	switch (ra->ra_type)
	{
	case RD_AVG_RATE:
		res->dbl = ((double)p->u.rate.cnt / (double)p->duration) *
			1000000.0f;
		res->sum = p->u.rate.cnt;
		break;

	case RD_AVG_HIST:
		/* No average for histograms, return 0. */
		goto no_data;
	}

	return res;
}


rd_avg_res_t rd_avg (rd_avg_t *ra, int period) {
	rd_avg_period_t *p;

	if (period == -1)
		period = ra->ra_curri;
	else if (period == -2) {
		if (ra->ra_curri == 0)
			period = ra->ra_periods-1;
		else
			period = (ra->ra_curri - 1) % ra->ra_periods;
	}
	
	p = &ra->ra_period[period];

	if (p == ra->ra_curr)
		p->duration = (p->last ? : rd_clock()) - ra->ra_start;

	return *(rd_avg_calc(ra, p));
}


static void rd_avg_period_next (rd_avg_t *ra, rd_ts_t now) {

	ra->ra_curri = (ra->ra_curri + 1) % ra->ra_periods;
	ra->ra_curr = &ra->ra_period[ra->ra_curri];

	ra->ra_start = now;
	ra->ra_end = ra->ra_start + ra->ra_duration;

	memset(ra->ra_curr, 0, sizeof(*ra->ra_curr));
	ra->ra_curr->res.low = RD_AVG_NO_VALUE;
	
}

void rd_avg_start (rd_avg_t *ra) {
	rd_avg_period_next(ra, rd_clock());
}



/**
 * Roll-over, stop and perform average calculations on the current period.
 * Set up a new current period.
 */
static void rd_avg_roll (rd_avg_t *ra, rd_ts_t now) {
	int missed;

	/* Calculate the average */
	ra->ra_curr->closed = now;
	ra->ra_curr->duration = (ra->ra_curr->last ? : now) - ra->ra_start;
	rd_avg_calc(ra, ra->ra_curr);
	
	/* Calculate missed periods */
	missed = (now - ra->ra_end) / ra->ra_duration;
	ra->ra_curr->missed = missed;

	/* Set up new current period */
	rd_avg_period_next(ra, now);
}

static inline void rd_avg_put_rate (rd_avg_t *ra, uint64_t val, rd_ts_t now) {
	ra->ra_curr->u.rate.cnt += val;
}

static inline void rd_avg_put_hist (rd_avg_t *ra, uint64_t val, rd_ts_t now) {
	int b;

	b = ra->ra_u.hist.val2bucket(ra, val, ra->ra_u.hist.buckets);
	if (unlikely(b == -1))
		return; /* Ignore data point */

	ra->ra_curr->u.hist.bucket[b]++;
}

void rd_avg_put (rd_avg_t *ra, uint64_t val) {
	rd_ts_t now = rd_clock();

	/* Check if current period has ended */
	if (ra->ra_end <= now)
		rd_avg_roll(ra, now);

	ra->ra_curr->last = now;

	if (ra->ra_curr->res.high < val)
		ra->ra_curr->res.high = val;
	if (ra->ra_curr->res.low > val)
		ra->ra_curr->res.low = val;

	switch (ra->ra_type)
	{
	case RD_AVG_RATE:
		return rd_avg_put_rate(ra, val, now);
	case RD_AVG_HIST:
		return rd_avg_put_hist(ra, val, now);
	}
}
