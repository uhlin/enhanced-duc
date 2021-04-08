#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "network.h"

#define FLAGS 0

static const char *hostname_array_match[] = {
	".noip.com",
	"noip.com",
	"www.noip.com",
};

static const char *hostname_array_mismatch[] = {
	".symantec.com",
	"symantec.com",
	"www.symantec.com",
};

static void
hostnameMatch_test(void **state)
{
	(void) state;

	assert_true(net_ssl_check_hostname(hostname_array_match[0], FLAGS) ==
	    HOSTNAME_MATCH);
	assert_true(net_ssl_check_hostname(hostname_array_match[1], FLAGS) ==
	    HOSTNAME_MATCH);
	assert_true(net_ssl_check_hostname(hostname_array_match[2], FLAGS) ==
	    HOSTNAME_MATCH);
}

static void
hostnameMismatch_test(void **state)
{
	(void) state;

	assert_true(net_ssl_check_hostname(hostname_array_mismatch[0], FLAGS) ==
	    HOSTNAME_MISMATCH);
	assert_true(net_ssl_check_hostname(hostname_array_mismatch[1], FLAGS) ==
	    HOSTNAME_MISMATCH);
	assert_true(net_ssl_check_hostname(hostname_array_mismatch[2], FLAGS) ==
	    HOSTNAME_MISMATCH);
}

int
main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(hostnameMatch_test),
		cmocka_unit_test(hostnameMismatch_test),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
