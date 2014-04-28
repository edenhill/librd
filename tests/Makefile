
# Makefile wrapper for expanding 0*.c glob.

.PHONY: all test clean testcontinue

all test clean:
	@echo "=================== SELFTESTS ======================="
	@(export TEST_SRCS="`echo 0*.c`"; make -B -f Makefile.tests $@)

testcontinue:
	@echo "=================== SELFTESTS (continue on fail) ===="
	@(export TEST_SRCS="`echo 0*.c`"; make -B -k -f Makefile.tests test)
