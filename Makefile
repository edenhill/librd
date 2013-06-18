
LIBNAME=librd
LIBVER=0
LIBVER_FULL=$(LIBVER).0.0

DESTDIR?=/usr/local

SRCS=	rd.c rdevent.c rdqueue.c rdthread.c rdtimer.c rdfile.c rdunits.c \
	rdlog.c rdbits.c rdopt.c rdmem.c rdaddr.c rdstring.c rdcrc32.c \
	rdgz.c rdrand.c rdbuf.c rdavl.c rdio.c rdencoding.c rdiothread.c \
	rdlru.c rdavg.c rdalert.c

HDRS=	rdbits.h rdevent.h rdfloat.h rd.h rdsysqueue.h rdqueue.h \
	rdsignal.h rdthread.h rdtime.h rdtimer.h rdtypes.h rdfile.h rdunits.h \
	rdlog.h rdopt.h rdmem.h rdaddr.h rdstring.h rdcrc32.h \
	rdgz.h rdrand.h rdbuf.h rdavl.h rdio.h rdencoding.h rdiothread.h \
	rdlru.h rdavg.h rdalert.h

OBJS=	$(SRCS:.c=.o)
DEPS=	${OBJS:%.o=%.d}

CFLAGS+=-Wformat -Werror=format-security -fno-stack-protector
CFLAGS+=-O2 -Wall -Werror -Wfloat-equal -Wpointer-arith -fPIC -I.
CFLAGS+=-g

# Profiling
#CFLAGS+=-O0
#CFLAGS += -pg
#LDFLAGS += -pg

LDFLAGS+=-g

.PHONY:

all: libs testcontinue

libs: $(LIBNAME).so.$(LIBVER) $(LIBNAME).a

%.o: %.c
	$(CC) -MD -MP $(CFLAGS) -c $<

$(LIBNAME).so.$(LIBVER): $(OBJS)
	$(LD) -fPIC -shared -soname,$@ \
		$(LDFLAGS) $(OBJS) -o $@
	ln -fs $(LIBNAME).so.$(LIBVER) $(LIBNAME).so 

$(LIBNAME).a:	$(OBJS)
	$(AR) rcs $@ $(OBJS)

testcontinue: .PHONY
	make -C tests $@

test: .PHONY
	make -C tests $@

install:
	install -d $(DESTDIR)/include/librd
	install -d $(DESTDIR)/lib
	install -d $(DESTDIR)/share/doc/librd$(LIBVER)
	install -t $(DESTDIR)/include/librd $(HDRS)
	install -t $(DESTDIR)/lib $(LIBNAME).so
	install -t $(DESTDIR)/lib $(LIBNAME).so.$(LIBVER)
	install -t $(DESTDIR)/lib $(LIBNAME).a
	install -t $(DESTDIR)/share/doc/librd$(LIBVER) README.markdown

clean:
	make -C tests clean
	rm -f $(OBJS) $(DEPS) $(LIBNAME).a $(LIBNAME).so $(LIBNAME).so.$(LIBVER)

-include $(DEPS)
