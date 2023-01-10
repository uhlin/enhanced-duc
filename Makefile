include options.mk

ROOT := ./
TGTS = enhanced-duc

all: $(TGTS)

include source/build.mk

.c.o:
	$(E) "  CC      " $@
	$(Q) $(CC) $(CFLAGS) -I include -c -o $@ $<
.cpp.o:
	$(E) "  CXX     " $@
	$(Q) $(CXX) $(CXXFLAGS) -I include -c -o $@ $<

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
