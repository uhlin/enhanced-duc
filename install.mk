INSTALL=install -D
INSTALL_DEPS=enhanced-duc enhanced-duc.1 example.conf
PREFIX?=/usr/local

DEST_PROGRAM=$(PREFIX)/bin/enhanced-duc
DEST_MANUAL=$(PREFIX)/man/man1
DEST_CONFIG=/etc/enhanced-duc.conf

install: $(INSTALL_DEPS)
	$(INSTALL) -m 0755 enhanced-duc $(DEST_PROGRAM)
	$(INSTALL) -m 0444 enhanced-duc.1 $(DEST_MANUAL)
	$(INSTALL) -m 0600 -b example.conf $(DEST_CONFIG)
