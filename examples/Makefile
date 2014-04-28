CC ?= cc
CFLAGS += -g
CFLAGS += -Wall -Werror -Wfloat-equal -Wpointer-arith -O2 -I../
LDFLAGS += -L../ ../librd.a
LDFLAGS += -lpthread -lrt -lz

# Profiling
#CFLAGS += -O0 -pg
#LDFLAGS += -pg

.PHONY: all clean

all:
	@echo "# Examples are built individually"
	@echo " make rdvarint"

rdvarint: rdvarint.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f rdvarint
