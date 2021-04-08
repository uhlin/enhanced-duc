#undef UNIT_TESTING

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "wrapper.h"

static void
canDuplicatePrintfStyleFormatString_test1(void **state)
{
	char *result = NULL;
	const char progname[] = "Enhanced DUC";
	const char month[] = "January";
	const int day = 5;
	const int year = 2016;

	result = strdup_printf("The first version of %s was released %s %d %d",
	    progname, month, day, year);
	assert_string_equal(result, "The first version of Enhanced DUC was "
	    "released January 5 2016");
	free(result);
}

static void
canDuplicatePrintfStyleFormatString_test2(void **state)
{
	char *result = NULL;
	const size_t size1 = 1000;
	const size_t size2 = 2000;
	const size_t size3 = 3000;

	result = strdup_printf("%zu->%zu->%zu", size1, size2, size3);
	assert_string_equal(result, "1000->2000->3000");
	free(result);
}

static void
canDuplicatePrintfStyleFormatString_test3(void **state)
{
	char *result = NULL;
	const double value1 = 0.99f;
	const double value2 = -10.0f;
	const double value3 = 13.31f;

	result = strdup_printf("%.2f %.2f %.2f %%", value1, value2, value3);
	assert_string_equal(result, "0.99 -10.00 13.31 %");
	free(result);
}

int
main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(canDuplicatePrintfStyleFormatString_test1),
		cmocka_unit_test(canDuplicatePrintfStyleFormatString_test2),
		cmocka_unit_test(canDuplicatePrintfStyleFormatString_test3),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
