include options.mk

ROOT := ./
TGTS = enhanced-duc

all: $(TGTS)

include build.mk

.c.o:
	$(E) "  CC      " $@
	$(Q) $(CC) $(CFLAGS) -c -o $@ $<
.cpp.o:
	$(E) "  CXX     " $@
	$(Q) $(CXX) $(CXXFLAGS) -c -o $@ $<

include tests/recompile.mk

check: $(OBJS)
	$(RM) $(RECOMPILE)
	$(Q) strip --strip-symbol=main main.o
	$(MAKE) -Ctests

# install target
include install.mk

clean:
	$(E) "  CLEAN"
	$(RM) $(OBJS)
	$(RM) $(TGTS)
	$(MAKE) -Ctests clean
