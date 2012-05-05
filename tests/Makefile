
CC ?= cc
TESTS ?= 0*.c
CFLAGS += -g
CFLAGS += -Wall -Werror -Wfloat-equal -O2 -I../
LDFLAGS += -L../ -lrd
LDFLAGS += -lpthread -lrt

all: tests


tests:
	@echo "=================== SELFTESTS ======================="
	@echo "Running tests: "
	@ls $(TESTS)
	@echo "-----------------------------------------------------"
	@(for t in $(TESTS); do \
		BIN=`echo $$t | sed 's/\.c$$//'`.test ; \
		LIBS=`grep '^//LIBS=' $$t| sed -e 's/^\/\/LIBS=//'` ; \
		if ! $(CC) $(CFLAGS) $$t -o $$BIN $(LDFLAGS) $$LIBS ; then \
			echo "Test $$t did not compile" ; \
			continue ; \
		fi ; \
		(LD_LIBRARY_PATH=../ ./$$BIN && echo "Test $$t passed") || \
			 (echo "########################################";\
			  echo "########################################";\
			  echo "Test $$t failed";\
			  echo "########################################";\
			  echo "########################################");\
		done \
	)
	@echo "====================== END =========================="


clean:
	@(for t in $(TESTS); do \
		BIN=`echo $$t | sed 's/\.c$$//'`.test ; \
		rm -f $$BIN ; \
		done)

