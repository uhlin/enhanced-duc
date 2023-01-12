# Makefile for use with BSD make

include options.mk

OSNAME != uname -s

.if $(OSNAME) == "Darwin"
CPPFLAGS += -I/usr/local/opt/libressl/include
LDFLAGS += -L/usr/local/opt/libressl/lib
.endif

# common vars
include vars.mk

all: $(TGTS)

include source/build.mk

# common rules
include common.mk

.PHONY: check clean install

include $(TARGETS_DIR)check.mk
include $(TARGETS_DIR)clean.mk
include $(TARGETS_DIR)install.mk
