# The 'check' target

include tests/recompile.mk

check: $(INCLUDE_DIR)funcs-yesno.h $(OBJS)
	$(RM) $(RECOMPILE)
	$(Q) strip --strip-symbol=main $(SRC_DIR)main.o
	$(MAKE) -Ctests
