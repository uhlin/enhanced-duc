# Enhanced DUC Makefile

E=@echo
Q=@

CC=gcc
DUC_CFLAGS=$(CFLAGS) -std=c99 -Wall -D_POSIX_C_SOURCE=200809L
OUT_NAME=educ_noip
LDFLAGS=-lcrypto -lssl
TEST_LDFLAGS=

RM=rm -f
INSTALL=install -D
PREFIX?=/usr/local
BIN_DIR=$(PREFIX)/bin
MAN_DIR=$(PREFIX)/man/man1
MAN_FILE=educ_noip.1

CONF_FILE=/etc/educ_noip.conf

OBJS=b64_decode.o
OBJS+=b64_encode.o
OBJS+=daemonize.o
OBJS+=interpreter.o
OBJS+=log.o
OBJS+=main.o
OBJS+=my_vasprintf.o
OBJS+=network-openssl.o
OBJS+=network.o
OBJS+=settings.o
OBJS+=sig.o
OBJS+=strlcat.o
OBJS+=strlcpy.o
OBJS+=various.o
OBJS+=wrapper.o

TEST_OBJS=interpreter.o log.o my_vasprintf.o ptest.o
TEST_OBJS+=settings.o tests.o various.o wrapper.o

.c.o:
	$(E) "  CC      " $@
	$(Q) $(CC) $(DUC_CFLAGS) -c $*.c

$(OUT_NAME): $(OBJS)
	$(E) "  LINK    " $@
	$(Q) $(CC) $(LDFLAGS) -o $(OUT_NAME) $(OBJS)

b64_decode.o: b64_decode.c
b64_encode.o: b64_encode.c
daemonize.o: daemonize.c
interpreter.o: interpreter.c
log.o: log.c
main.o: main.c
my_vasprintf.o: my_vasprintf.c
network-openssl.o: network-openssl.c
network.o: network.c
settings.o: settings.c
sig.o: sig.c
strlcat.o: strlcat.c
strlcpy.o: strlcpy.c
various.o: various.c
wrapper.o: wrapper.c

ptest.o: deps/ptest/ptest.c
	$(E) "  CC      " $@
	$(Q) $(CC) $(DUC_CFLAGS) -I. -Ideps/ptest -c deps/ptest/ptest.c -o ptest.o
tests.o: tests/tests.c
	$(E) "  CC      " $@
	$(Q) $(CC) $(DUC_CFLAGS) -I. -Ideps/ptest -c tests/tests.c -o tests.o

test: $(TEST_OBJS)
	$(E) "  LINK    " $@
	$(Q) $(CC) $(TEST_LDFLAGS) -o $@ $(TEST_OBJS)
	./$@

.PHONY: config-test clean install

config-test: $(OUT_NAME)
	./config-test.sh

clean:
	$(E) "  CLEAN"
	$(Q) $(RM) $(OUT_NAME) $(OBJS) test ptest.o tests.o

install: $(OUT_NAME) $(MAN_FILE) example.conf
	$(E) "  INSTALL " $(BIN_DIR)/$(OUT_NAME)
	$(Q) $(INSTALL) -m 0755 $(OUT_NAME) $(BIN_DIR)/$(OUT_NAME)
	$(E) "  INSTALL " $(MAN_DIR)/$(MAN_FILE)
	$(Q) $(INSTALL) -m 0444 $(MAN_FILE) $(MAN_DIR)/$(MAN_FILE)
	$(E) "  INSTALL " $(CONF_FILE)
	$(Q) $(INSTALL) -m 0600 -b example.conf $(CONF_FILE)
