# Makefile for use with GNU make

include options.mk

OSNAME := $(shell uname -s)

ifeq ($(OSNAME),Darwin)
CPPFLAGS += -I/usr/local/opt/libressl/include
LDFLAGS += -L/usr/local/opt/libressl/lib
endif

# common vars
include vars.mk

all: $(TGTS)

include source/build.mk

# common rules
include common.mk

include $(TARGETS_DIR)check.mk
include $(TARGETS_DIR)clean.mk
include $(TARGETS_DIR)install.mk