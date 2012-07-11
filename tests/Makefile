
# Makefile wrapper for expanding 0*.c glob.

all tests clean:
	@echo "=================== SELFTESTS ======================="
	@(export TEST_SRCS="`echo 0*.c`"; make -B -f Makefile.tests $@)

testscontinue:
	@echo "=================== SELFTESTS (continue on fail) ===="
	@(export TEST_SRCS="`echo 0*.c`"; make -B -k -f Makefile.tests tests)
