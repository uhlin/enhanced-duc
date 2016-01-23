# Enhanced DUC Makefile

E=@echo
Q=@

CC=gcc
CFLAGS=-std=c99 -Wall -D_POSIX_C_SOURCE=200809L
OUT_NAME=educ_noip
LDFLAGS=-lcrypto -lssl

RM=rm -f
INSTALL=install
BIN_DIR=/usr/local/bin

OBJS=b64_decode.o b64_encode.o daemonize.o duc_strlcat.o duc_strlcpy.o
OBJS+=interpreter.o log.o main.o my_vasprintf.o network.o
OBJS+=network-openssl.o settings.o sig.o various.o wrapper.o

.c.o:
	$(E) "  CC      " $@
	$(Q) $(CC) $(CFLAGS) -c $*.c

$(OUT_NAME): $(OBJS)
	$(E) "  LINK    " $@
	$(Q) $(CC) $(LDFLAGS) -o $(OUT_NAME) $(OBJS)

b64_decode.o: b64_decode.c
b64_encode.o: b64_encode.c
daemonize.o: daemonize.c
duc_strlcat.o: duc_strlcat.c
duc_strlcpy.o: duc_strlcpy.c
interpreter.o: interpreter.c
log.o: log.c
main.o: main.c
my_vasprintf.o: my_vasprintf.c
network.o: network.c
network-openssl.o: network-openssl.c
settings.o: settings.c
sig.o: sig.c
various.o: various.c
wrapper.o: wrapper.c

clean:
	$(E) "  CLEAN"
	$(Q) $(RM) $(OUT_NAME) $(OBJS)

install: $(OUT_NAME)
	$(E) "  INSTALL " $(OUT_NAME)
	$(Q) $(INSTALL) $(OUT_NAME) $(BIN_DIR)/$(OUT_NAME)
