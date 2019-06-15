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
	$(CC) $(CFLAGS) -c -o $@ $<

.o.run:
	$(CXX) $(CXXFLAGS) -o $@ $*.o $(OBJS) $(LDFLAGS)
	strip $@

all: $(OBJS) $(TEST_OBJS) $(TESTS)

clean:
	$(RM) $(OBJS)
	$(RM) $(TEST_OBJS)
	$(RM) $(TESTS)
