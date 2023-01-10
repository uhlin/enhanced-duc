# options.mk

CC = cc
CFLAGS = -D_BSD_SOURCE=1\
	-D_DEFAULT_SOURCE=1\
	-D_POSIX_C_SOURCE=200809L\
	-O2\
	-Wall\
	-Wsign-compare\
	-Wstrict-prototypes\
	-pipe\
	-std=c11

CXX = c++
CXXCLAGS = -std=c++17

LDFLAGS = -lcrypto -lssl
RM ?= @rm -f

E = @echo
Q = @
