include options.mk

TGTS=enhanced-duc

all: $(TGTS)

include build.mk

.c.o:
	@echo "  CC      " $@
	@$(CC) $(CFLAGS) -c -o $@ $<
.cpp.o:
	@echo "  CXX     " $@
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

# install target
include install.mk

clean:
	@echo "  CLEAN"
	@$(RM) $(OBJS)
	@$(RM) $(TGTS)
