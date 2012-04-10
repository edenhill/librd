
LIBNAME=librd.so
LIBVER=	0
LIBVER_FULL=0.0.0

# Use gcc as ld to avoid __stack_chk_fail_error symbol error.
LD=gcc

SRCS=	rd.c rdqueue.c rdthread.c rdtimer.c rdfile.c rdunits.c \
	rdlog.c rdbits.c rdopt.c rdmem.c
HDRS=	rdbits.h rdevent.h rdfloat.h rd.h rdsysqueue.h rdqueue.h \
	rdsignal.h rdthread.h rdtime.h rdtimer.h rdtypes.h rdfile.h rdunits.h \
	rdlog.h rdopt.h rdmem.h

OBJS=	$(SRCS:.c=.o)

CFLAGS+=-O2 -Wall -Werror -fPIC
CFLAGS+=-g

LDFLAGS+=-shared -g -fPIC -lpthread -lrt -lc

.PHONY:

all: $(LIBNAME) tests


%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(LIBNAME):	$(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(LIBNAME)

tests: .PHONY
	make -C tests

clean:
	make -C tests clean
	rm -f $(OBJS) $(LIBNAME)
