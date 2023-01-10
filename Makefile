include options.mk

ROOT := ./
INCLUDE_DIR := $(ROOT)include/
TGTS = enhanced-duc

all: $(TGTS)

include source/build.mk

.c.o:
	$(E) "  CC      " $@
	$(Q) $(CC) $(CFLAGS) -I $(INCLUDE_DIR) -c -o $@ $<
.cpp.o:
	$(E) "  CXX     " $@
	$(Q) $(CXX) $(CXXFLAGS) -I $(INCLUDE_DIR) -c -o $@ $<

include tests/recompile.mk

check: $(OBJS)
	$(RM) $(RECOMPILE)
	$(Q) strip --strip-symbol=main $(SRC_DIR)main.o
	$(MAKE) -Ctests

# install target
include install.mk

clean:
	$(E) "  CLEAN"
	$(RM) $(OBJS)
	$(RM) $(TGTS)
	$(MAKE) -Ctests clean
