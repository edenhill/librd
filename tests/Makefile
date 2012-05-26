
CC ?= cc
TESTS ?= 0*.c
CFLAGS += -g
CFLAGS += -Wall -Werror -Wfloat-equal -O2 -I../
LDFLAGS += -L../ -lrd
LDFLAGS += -lpthread -lrt -lz

# Profiling
#CFLAGS += -O0 -pg
#LDFLAGS += -pg

all: tests


tests:
	@echo "=================== SELFTESTS ======================="
	@echo "Running tests: "
	@ls $(TESTS)
	@echo "-----------------------------------------------------"
	@(FAILS=""; \
	  (for t in $(TESTS); do \
		BIN=`echo $$t | sed 's/\.c$$//'`.test ; \
		LIBS=`grep '^//LIBS=' $$t| sed -e 's/^\/\/LIBS=//'` ; \
		if ! $(CC) $(CFLAGS) $$t -o $$BIN $(LDFLAGS) $$LIBS ; then \
			echo "Test $$t did not compile" ; \
			FAILS="$$FAILS $$t" ; \
			continue ; \
		fi ; \
		echo -n "Test $$t "; \
		(LD_LIBRARY_PATH=../ ./$$BIN && echo "passed") || \
			 (echo "failed";\
			  echo "########################################";\
			  echo "########################################";\
			  echo "Test $$t failed";\
			  echo "########################################";\
			  echo "########################################";\
			  FAILS="$$FAILS $$t");\
	    done) ; echo "Fails: $$FAILS" )
	@echo "====================== END =========================="


clean:
	@(for t in $(TESTS); do \
		BIN=`echo $$t | sed 's/\.c$$//'`.test ; \
		rm -f $$BIN ; \
		done)

