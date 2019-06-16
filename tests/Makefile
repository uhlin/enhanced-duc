ROOT=..

include $(ROOT)/options.mk

CFLAGS+=-DUNIT_TESTING=1
CFLAGS+=-I$(ROOT)
CFLAGS+=-I/usr/local/include
LDFLAGS+=-L/usr/local/lib
LDFLAGS+=-lcmocka

include build.mk

.SUFFIXES: .c .o .run

.c.o:
	@echo "  CC      " $@
	@$(CC) $(CFLAGS) -c -o $@ $<
.cpp.o:
	@echo "  CXX     " $@
	@$(CXX) $(CXXFLAGS) -c -o $@ $<
.o.run:
	@echo "  LINK    " $@
	@$(CXX) $(CXXFLAGS) -o $@ $*.o $(OBJS) $(LDFLAGS)
	@strip $@

all: $(OBJS) $(TEST_OBJS) $(TESTS)

clean:
	@echo "  CLEAN"
	@$(RM) $(OBJS)
	@$(RM) $(TEST_OBJS)
	@$(RM) $(TESTS)
