# options.mk

CC ?= cc
CFLAGS = -O2\
	-Wall\
	-Wformat-security\
	-Wshadow\
	-Wsign-compare\
	-Wstrict-prototypes\
	-pipe\
	-std=c11

CXX ?= c++
CXXFLAGS = -std=c++17

# C preprocessor flags
CPPFLAGS = -D_BSD_SOURCE=1\
	-D_DEFAULT_SOURCE=1\
	-D_POSIX_C_SOURCE=200809L

LDFLAGS =
LDLIBS = -lcrypto -lssl
RM ?= @rm -f

E = @echo
Q = @
