include options.mk

all: enhanced-duc

include build.mk

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS)
	$(RM) $(TGTS)
