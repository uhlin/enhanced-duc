# common.mk  --  common rules

$(INCLUDE_DIR)funcs-yesno.h:
	$(ROOT)check-funcs.sh "$(INCLUDE_DIR)funcs-yesno.h"

.c.o:
	$(E) "  CC      " $@
	$(Q) $(CC) $(CFLAGS) -I $(INCLUDE_DIR) $(CPPFLAGS) -c -o $@ $<
.cpp.o:
	$(E) "  CXX     " $@
	$(Q) $(CXX) $(CXXFLAGS) -I $(INCLUDE_DIR) $(CPPFLAGS) -c -o $@ $<
