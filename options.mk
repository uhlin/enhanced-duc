CC=cc
CFLAGS=\
	-D_POSIX_C_SOURCE=200809L\
	-Wall\
	-std=c99
CXX=c++
CXXCLAGS=
LDFLAGS=\
	-lcrypto\
	-lssl
RM?=rm -f
