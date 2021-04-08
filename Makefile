include options.mk

TGTS=enhanced-duc

all: $(TGTS)

include build.mk

.c.o:
	$(E) "  CC      " $@
	$(Q) $(CC) $(CFLAGS) -c -o $@ $<
.cpp.o:
	$(E) "  CXX     " $@
	$(Q) $(CXX) $(CXXFLAGS) -c -o $@ $<

check: $(OBJS)
	$(Q) strip --strip-symbol=main main.o
	$(MAKE) -Ctests

# install target
include install.mk

clean:
	$(E) "  CLEAN"
	$(Q) $(RM) $(OBJS)
	$(Q) $(RM) $(TGTS)
