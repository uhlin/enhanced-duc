# The 'tidy' target

CHKS = cert-*,$\
	-clang-analyzer-security.insecureAPI.strcpy,$\
	-clang-analyzer-optin.performance.Padding,$\
	-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling,$\
	cppcoreguidelines-prefer-member-initializer,$\
	hicpp-deprecated-headers,$\
	hicpp-multiway-paths-covered,$\
	hicpp-special-member-functions,$\
	hicpp-use-auto,$\
	hicpp-use-emplace,$\
	hicpp-use-equals-default,$\
	hicpp-use-equals-delete,$\
	hicpp-use-nullptr,$\
	performance-*,$\
	portability-*

TIDY = clang-tidy
TIDYFLAGS = -checks=$(CHKS) -quiet
DUC_TIDYFLAGS ?=

tidy: $(INCLUDE_DIR)funcs-yesno.h
	$(TIDY) $(SRC_FILES) $(TIDYFLAGS) $(DUC_TIDYFLAGS) -- \
	-I $(INCLUDE_DIR) $(CPPFLAGS)
