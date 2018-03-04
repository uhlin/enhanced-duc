CC=gcc
CFLAGS=\
	-D_POSIX_C_SOURCE=200809L\
	-Wall\
	-std=c99
LDFLAGS=\
	-lcrypto\
	-lssl
