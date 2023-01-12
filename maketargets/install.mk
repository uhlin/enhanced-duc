# The 'install' target

INSTALL = install
INSTALL_DEPS = enhanced-duc enhanced-duc.1 template.conf
PREFIX ?= /usr/local

# Don't provide a default value for DESTDIR. It should be empty.
DESTDIR ?=

DEST_PROGRAM = $(DESTDIR)$(PREFIX)/bin
DEST_MANUAL = $(DESTDIR)$(PREFIX)/share/man/man1
DEST_CONFIG = $(DESTDIR)/etc/enhanced-duc.template.conf

install: $(INSTALL_DEPS)
	$(INSTALL) -d $(DEST_PROGRAM)
	$(INSTALL) -d $(DEST_MANUAL)
	$(INSTALL) -m 0755 enhanced-duc $(DEST_PROGRAM)
	$(INSTALL) -m 0444 enhanced-duc.1 $(DEST_MANUAL)
	$(INSTALL) -m 0600 -b template.conf $(DEST_CONFIG)
