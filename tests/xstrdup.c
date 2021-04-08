#undef UNIT_TESTING

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "wrapper.h"

static void
canDuplicateString_test1(void **state)
{
	(void) state;

	const char string[] = "Hello World! Unit testing in progress...";
	char *result = NULL;

	result = xstrdup(string);
	assert_string_equal(result, string);
	free(result);
}

static void
canDuplicateString_test2(void **state)
{
	(void) state;

	const char string[] = "Hello world! My name is Markus.\r\n";
	char *result = NULL;

	result = xstrdup(string);
	assert_string_equal(result, string);
	free(result);
}

int
main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(canDuplicateString_test1),
		cmocka_unit_test(canDuplicateString_test2),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
