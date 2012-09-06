
LIBNAME=librd
LIBVER=0
LIBVER_FULL=$(LIBVER).0.0


PREFIX?=/usr/local

# Use gcc as ld to avoid __stack_chk_fail_error symbol error.
LD=gcc

SRCS=	rd.c rdqueue.c rdthread.c rdtimer.c rdfile.c rdunits.c \
	rdlog.c rdbits.c rdopt.c rdmem.c rdaddr.c rdstring.c rdcrc32.c \
	rdgz.c rdrand.c rdbuf.c rdavl.c rdio.c rdencoding.c \
	rdkafka.c

HDRS=	rdbits.h rdevent.h rdfloat.h rd.h rdsysqueue.h rdqueue.h \
	rdsignal.h rdthread.h rdtime.h rdtimer.h rdtypes.h rdfile.h rdunits.h \
	rdlog.h rdopt.h rdmem.h rdaddr.h rdstring.h rdcrc32.h \
	rdgz.h rdrand.h rdbuf.h rdavl.h rdio.h rdencoding.h \
	rdkafka.h

OBJS=	$(SRCS:.c=.o)
DEPS=	${OBJS:%.o=%.d}

CFLAGS+=-O2 -Wall -Werror -Wfloat-equal -Wpointer-arith -fPIC -I.
CFLAGS+=-g

# Profiling
#CFLAGS+=-O0
#CFLAGS += -pg
#LDFLAGS += -pg

LDFLAGS+=-shared -g -fPIC -lpthread -lrt -lz -lc

.PHONY:

all: libs testscontinue

libs: $(LIBNAME).so $(LIBNAME).a

%.o: %.c
	$(CC) -MD -MP $(CFLAGS) -c $<

$(LIBNAME).so:	$(OBJS)
	$(LD) -shared -Wl,-soname,$(LIBNAME).so.$(LIBVER) \
		$(LDFLAGS) $(OBJS) -o $@
	ln -fs $(LIBNAME).so $(LIBNAME).so.$(LIBVER)

$(LIBNAME).a:	$(OBJS)
	$(AR) rcs $@ $(OBJS)

testscontinue: .PHONY
	make -C tests $@

tests: .PHONY
	make -C tests $@

install:
	install -d $(PREFIX)/include/librd $(PREFIX)/lib
	install -t $(PREFIX)/include/librd $(HDRS)
	install -t $(PREFIX)/lib $(LIBNAME).so
	install -t $(PREFIX)/lib $(LIBNAME).so.$(LIBVER)
	install -t $(PREFIX)/lib $(LIBNAME).a

clean:
	make -C tests clean
	rm -f $(OBJS) $(DEPS) $(LIBNAME)

-include $(DEPS)
