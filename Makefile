include options.mk

ROOT := ./
INCLUDE_DIR := $(ROOT)include/
TGTS = $(INCLUDE_DIR)funcs-yesno.h\
	enhanced-duc

all: $(TGTS)

include source/build.mk

# common rules
include common.mk

# check target
include check.mk

# clean target
include clean.mk

# install target
include install.mk
