# Enhanced DUC Makefile

E=@echo
Q=@

CC=gcc
CFLAGS=-std=c99 -Wall -D_POSIX_C_SOURCE=200809L
OUT_NAME=educ_noip
LDFLAGS=

RM=rm -f
INSTALL=install
BIN_DIR=/usr/local/bin

OBJS=b64_encode.o interpreter.o log.o
OBJS+=main.o my_vasprintf.o network.o
OBJS+=sig.o various.o wrapper.o

.c.o:
	$(E) "  CC      " $@
	$(Q) $(CC) $(CFLAGS) -c $*.c

$(OUT_NAME): $(OBJS)
	$(E) "  LINK    " $@
	$(Q) $(CC) $(LDFLAGS) -o $(OUT_NAME) $(OBJS)

b64_encode.o: b64_encode.c
interpreter.o: interpreter.c
log.o: log.c
main.o: main.c
my_vasprintf.o: my_vasprintf.c
network.o: network.c
sig.o: sig.c
various.o: various.c
wrapper.o: wrapper.c

clean:
	$(E) "  CLEAN"
	$(Q) $(RM) $(OUT_NAME) $(OBJS)

install: $(OUT_NAME)
	$(E) "  INSTALL " $(OUT_NAME)
	$(Q) $(INSTALL) $(OUT_NAME) $(BIN_DIR)/$(OUT_NAME)
