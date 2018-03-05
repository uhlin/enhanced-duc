ROOT=..

include $(ROOT)/options.mk

CFLAGS+=-DUNIT_TESTING=1
CFLAGS+=-I$(ROOT)
LDFLAGS+=-lcmocka

include build.mk

.SUFFIXES: .c .o .run

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

.o.run:
	$(CC) $(CFLAGS) -o $@ $*.o $(OBJS) $(LDFLAGS)
	strip $@

all: $(OBJS) $(TEST_OBJS) $(TESTS)

clean:
	$(RM) $(OBJS)
	$(RM) $(TEST_OBJS)
	$(RM) $(TESTS)
