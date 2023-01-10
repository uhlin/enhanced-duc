# options.mk

CC = cc
CFLAGS = -D_POSIX_C_SOURCE=200809L\
	-O2\
	-Wall\
	-Wsign-compare\
	-Wstrict-prototypes\
	-pipe\
	-std=c11

CXX = c++
CXXCLAGS =

LDFLAGS = -lcrypto -lssl
RM ?= @rm -f

E = @echo
Q = @
