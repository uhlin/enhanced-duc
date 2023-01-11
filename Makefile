include options.mk

ROOT := ./
INCLUDE_DIR := $(ROOT)include/
TGTS = $(INCLUDE_DIR)funcs-yesno.h\
	enhanced-duc

all: $(TGTS)

include source/build.mk

$(INCLUDE_DIR)funcs-yesno.h:
	$(ROOT)check-funcs.sh "$(INCLUDE_DIR)funcs-yesno.h"

.c.o:
	$(E) "  CC      " $@
	$(Q) $(CC) $(CFLAGS) -I $(INCLUDE_DIR) -c -o $@ $<
.cpp.o:
	$(E) "  CXX     " $@
	$(Q) $(CXX) $(CXXFLAGS) -I $(INCLUDE_DIR) -c -o $@ $<

# check target
include check.mk

# install target
include install.mk

# clean target
include clean.mk
