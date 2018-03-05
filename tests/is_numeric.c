#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "various.h"

static void
returnsFalse_test1(void **state)
{
    assert_false(is_numeric("0123456789 "));
}

static void
returnsFalse_test2(void **state)
{
    assert_false(is_numeric(" 0123456789"));
}

static void
returnsFalse_test3(void **state)
{
    assert_false(is_numeric("123abc"));
}

static void
returnsFalseIfStringIsNull(void **state)
{
    assert_false(is_numeric(NULL));
}

static void
returnsFalseIfStringIsEmpty(void **state)
{
    assert_false(is_numeric(""));
}

static void
returnsTrue_test1(void **state)
{
    assert_true(is_numeric("0123456789"));
}

static void
returnsTrue_test2(void **state)
{
    assert_true(is_numeric("122333"));
}

static void
returnsTrue_test3(void **state)
{
    assert_true(is_numeric("0"));
}

int
main()
{
    const struct CMUnitTest tests[] = {
	cmocka_unit_test(returnsFalse_test1),
	cmocka_unit_test(returnsFalse_test2),
	cmocka_unit_test(returnsFalse_test3),
	cmocka_unit_test(returnsFalseIfStringIsNull),
	cmocka_unit_test(returnsFalseIfStringIsEmpty),
	cmocka_unit_test(returnsTrue_test1),
	cmocka_unit_test(returnsTrue_test2),
	cmocka_unit_test(returnsTrue_test3),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
