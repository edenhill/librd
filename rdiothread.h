#pragma once

#include "rdthread.h"

#include <sys/epoll.h>


int rd_io_del (int fd);

#define RD_IO_F_NONBLOCKING   0x1  /* Handler promises only to perform
				    * nonblocking ops. */

int rd_io_add (int fd, int events, int flags, rd_thread_t *target_thread,
	       void (*handler) (int fd, int events,
				rd_thread_t *target_thread, void *opaque),
	       void *opaque);
