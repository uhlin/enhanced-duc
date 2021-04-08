#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "various.h"

static char dst[100] = "";

static void
returnsAnEmptyStringOnEmptyInput(void **state)
{
    strlcpy(dst, "", sizeof dst);
    assert_string_equal(trim(dst), "");
}

static void
returnsAnEmptyStringIfInputIsAllWhitespace(void **state)
{
    strlcpy(dst, "     \f\n\r\t\t\t\t\t\v     ", sizeof dst);
    assert_string_equal(trim(dst), "");
}

static void
canTrim_test1(void **state)
{
    strlcpy(dst, "hello, world!\r\n", sizeof dst);
    assert_string_equal(trim(dst), "hello, world!");
}

static void
canTrim_test2(void **state)
{
    strlcpy(dst, "hello, world!\r", sizeof dst);
    assert_string_equal(trim(dst), "hello, world!");
}

static void
canTrim_test3(void **state)
{
    strlcpy(dst, "hello, world!\n", sizeof dst);
    assert_string_equal(trim(dst), "hello, world!");
}

static void
canTrim_test4(void **state)
{
    strlcpy(dst, "hello, world!     \f\n\r\t\t\t\t\t\v     ", sizeof dst);
    assert_string_equal(trim(dst), "hello, world!");
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
	cmocka_unit_test(returnsAnEmptyStringOnEmptyInput),
	cmocka_unit_test(returnsAnEmptyStringIfInputIsAllWhitespace),
	cmocka_unit_test(canTrim_test1),
	cmocka_unit_test(canTrim_test2),
	cmocka_unit_test(canTrim_test3),
	cmocka_unit_test(canTrim_test4),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
