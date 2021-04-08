ROOT = ../

TEST_LOGFILE ?= /tmp/run.log

include $(ROOT)options.mk

CFLAGS += -DUNIT_TESTING=1\
	-I$(ROOT)\
	-I/usr/local/include

LDFLAGS += -L/usr/local/lib\
	-lcmocka

all: main

include tests.mk

main: $(TESTS)

.SUFFIXES: .c .o .run

.c.o:
	$(E) "  CC      " $@
	$(Q) $(CC) $(CFLAGS) -c -o $@ $<

.cpp.o:
	$(E) "  CXX     " $@
	$(Q) $(CXX) $(CXXFLAGS) -c -o $@ $<

.o.run:
	$(E) "  LINK    " $@
	$(Q) $(CXX) $(CXXFLAGS) -o $@ $*.o $(ROOT)*.o $(LDFLAGS)
	$(Q) strip $@

clean:
	$(E) "  CLEAN"
	$(RM) "$(TEST_LOGFILE)"
	$(RM) $(TESTS)
	$(RM) *.o
