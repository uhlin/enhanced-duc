# The 'clean' target

clean:
	$(E) "  CLEAN"
	$(RM) $(OBJS)
	$(RM) $(TGTS)
	$(MAKE) -Ctests clean
