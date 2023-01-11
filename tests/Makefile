ROOT := ../
INCLUDE_DIR := $(ROOT)include/
SRC_DIR := $(ROOT)source/

TEST_LOGFILE ?= /tmp/run.log

include $(ROOT)options.mk

CFLAGS += -DUNIT_TESTING=1\
	-I$(ROOT)\
	-I/usr/local/include

LDFLAGS += -L/usr/local/lib\
	-lcmocka

all: main

include recompile.mk
include tests.mk

main: $(RECOMPILE) $(TESTS)
	./run-all-tests > "$(TEST_LOGFILE)"
	$(E) "$(TEST_LOGFILE) written"

.SUFFIXES: .c .o .run

.c.o:
	$(E) "  CC      " $@
	$(Q) $(CC) $(CFLAGS) -I $(INCLUDE_DIR) -I $(SRC_DIR) -c -o $@ $<

.o.run:
	$(E) "  LINK    " $@
	$(Q) $(CXX) $(CXXFLAGS) -o $@ $*.o $(SRC_DIR)*.o $(LDFLAGS) $(LDLIBS)
	$(Q) strip $@

clean:
	$(E) "  CLEAN"
	$(RM) "$(TEST_LOGFILE)"
	$(RM) $(TESTS)
	$(RM) *.o
