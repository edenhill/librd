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



typedef enum {
	/* A thread has stalled its operation.
	 * (May be legit due to a long-running operation).
	 *
	 * Locality: any thread
	 * Arguments:
	 *   rd_thread_t *stalling_thread
	 *   int stall_time    <-- in seconds, may be 0 if unknown.
	 */
	RD_ALERT_THREAD_STALL,

	/* Used for registration: subscribe to all alert types. */
	RD_ALERT_ALL,
} rd_alert_type_t;



void rd_alert0 (const char *file, const char *func, int line,
		rd_alert_type_t type, int level, const char *reason, ...);

#define rd_alert(type, level...) \
	rd_alert0(__FILE__,__FUNCTION__,__LINE__,type, level)




/**
 * Register callback for alert type 'type' (or all types if type
 * is RD_ALERT_ALL).
 * The callbacks will be called with the fixed arguments as well as the
 * type-specific arguments documented above in rd_alert_type_t.
 *
 * NOTE: A callback must not call rd_alert_register().
 *
 * Any number of callbacks can be registered.
 */
void rd_alert_register (rd_alert_type_t type,
			void (*callback) (rd_alert_type_t type, int level,
					  const char *reason, void *opaque,
					  ...),
			void *opaque);
