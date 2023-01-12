SRC_DIR := source/

OBJS = $(SRC_DIR)b64_decode.o\
	$(SRC_DIR)b64_encode.o\
	$(SRC_DIR)daemonize.o\
	$(SRC_DIR)interpreter.o\
	$(SRC_DIR)log.o\
	$(SRC_DIR)main.o\
	$(SRC_DIR)my_vasprintf.o\
	$(SRC_DIR)network-openssl.o\
	$(SRC_DIR)network.o\
	$(SRC_DIR)settings.o\
	$(SRC_DIR)sig.o\
	$(SRC_DIR)strlcat.o\
	$(SRC_DIR)strlcpy.o\
	$(SRC_DIR)terminate.o\
	$(SRC_DIR)various.o\
	$(SRC_DIR)wrapper.o

enhanced-duc: $(OBJS)
	$(E) "  LINK    " $@
	$(Q) $(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
