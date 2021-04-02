OBJS = b64_decode.o\
	b64_encode.o\
	daemonize.o\
	interpreter.o\
	log.o\
	main.o\
	my_vasprintf.o\
	network.o\
	network-openssl.o\
	settings.o\
	sig.o\
	strlcat.o\
	strlcpy.o\
	various.o\
	wrapper.o

enhanced-duc: $(OBJS)
	@echo "  LINK    " $@
	@$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)
