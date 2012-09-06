#pragma once


#include <poll.h>

#define RD_POLL_INFINITE  -1
#define RD_POLL_NOWAIT     0


/**
 * Poll a single 'fd' for 'events' with timeout 'timeout_ms' according to
 * poll(2) behaviour.
 * Returns the revents bits, or 0 on no events, or -1 on failure.
 */
int rd_io_poll_single (int fd, short events, int timeout_ms);
