
CC ?= cc
TESTS ?= 0*.c
CFLAGS += -g
CFLAGS += -Wall -Werror -O2 -I../
LDFLAGS += -L../ -lrd
#LDFLAGS += -lpthread -lrt

all: tests


tests:
	@echo "=================== SELFTESTS ======================="
	@echo "Running tests: "
	@ls $(TESTS)
	@echo "-----------------------------------------------------"
	@(for t in $(TESTS); do \
		BIN=`echo $$t | sed 's/\.c$$//'`.test ; \
		if ! $(CC) $(CFLAGS) $(LDFLAGS) $$t -o $$BIN ; then \
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

