include options.mk

ROOT := ./
INCLUDE_DIR := $(ROOT)include/
TARGETS_DIR := $(ROOT)maketargets/
TGTS = $(INCLUDE_DIR)funcs-yesno.h\
	enhanced-duc

all: $(TGTS)

include source/build.mk

# common rules
include common.mk

include $(TARGETS_DIR)check.mk
include $(TARGETS_DIR)clean.mk
include $(TARGETS_DIR)install.mk
