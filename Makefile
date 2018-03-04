include options.mk

TGTS=enhanced-duc

all: $(TGTS)

include build.mk

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS)
	$(RM) $(TGTS)
