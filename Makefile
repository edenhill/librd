
LIBNAME=librd.so
LIBVER=	0
LIBVER_FULL=0.0.0

# Use gcc as ld to avoid __stack_chk_fail_error symbol error.
LD=gcc

SRCS=	rd.c rdqueue.c rdthread.c rdtimer.c rdfile.c rdunits.c \
	rdlog.c rdbits.c rdopt.c rdmem.c rdaddr.c rdstring.c rdcrc32.c \
	rdgz.c rdrand.c
HDRS=	rdbits.h rdevent.h rdfloat.h rd.h rdsysqueue.h rdqueue.h \
	rdsignal.h rdthread.h rdtime.h rdtimer.h rdtypes.h rdfile.h rdunits.h \
	rdlog.h rdopt.h rdmem.h rdaddr.h rdstring.h rdcrc32.h \
	rdgz.h rdrand.h

OBJS=	$(SRCS:.c=.o)
DEPS=	${OBJS:%.o=%.d}

CFLAGS+=-O2 -Wall -Werror -Wfloat-equal -fPIC -I.
CFLAGS+=-g

# Profiling
#CFLAGS+=-O0
#CFLAGS += -pg
#LDFLAGS += -pg

LDFLAGS+=-shared -g -fPIC -lpthread -lrt -lz -lc

.PHONY:

all: $(LIBNAME) tests


%.o: %.c
	$(CC) -MD -MP $(CFLAGS) -c $<

$(LIBNAME):	$(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(LIBNAME)

tests: .PHONY
	make -C tests

clean:
	make -C tests clean
	rm -f $(OBJS) $(DEPS) $(LIBNAME)

-include $(DEPS)
