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
    assert_string_equal(strToLower(dst), "");
}

static void
ignoresNonUppercaseChars(void **state)
{
    strlcpy(dst, "     \f\n\r\t\t\t\t\t\v     ", sizeof dst);
    assert_string_equal(strToLower(dst), "     \f\n\r\t\t\t\t\t\v     ");
}

static void
canLowercaseString_test1(void **state)
{
    strlcpy(dst, "HELLO, WORLD!", sizeof dst);
    assert_string_equal(strToLower(dst), "hello, world!");
}

static void
canLowercaseString_test2(void **state)
{
    strlcpy(dst, "MAKE ME\r\nto ALL\r\nLOWER", sizeof dst);
    assert_string_equal(strToLower(dst), "make me\r\nto all\r\nlower");
}

static void
canLowercaseString_test3(void **state)
{
    strlcpy(dst, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", sizeof dst);
    assert_string_equal(strToLower(dst), "abcdefghijklmnopqrstuvwxyz");
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
	cmocka_unit_test(returnsAnEmptyStringOnEmptyInput),
	cmocka_unit_test(ignoresNonUppercaseChars),
	cmocka_unit_test(canLowercaseString_test1),
	cmocka_unit_test(canLowercaseString_test2),
	cmocka_unit_test(canLowercaseString_test3),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
