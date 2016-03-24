#include <stdio.h>

#include "ptest.h"
#include "various.h"
#include "wrapper.h"

PT_SUITE(suite_various)
{
    PT_TEST(test_size_product)
    {
	PT_ASSERT(size_product(3, 3) == 9);
	PT_ASSERT(size_product(5, 20) == 100);
	PT_ASSERT(size_product(0, 1000) == 0);
    }
    PT_TEST(test_is_numeric)
    {
	char *str;

	str = "123";
	PT_ASSERT(is_numeric(str));

	str = "123456 789";
	PT_ASSERT(!is_numeric(str));

	str = " 123456789";
	PT_ASSERT(!is_numeric(str));

	str = "123456789 ";
	PT_ASSERT(!is_numeric(str));
    }
    PT_TEST(test_trim)
    {
	char buf[200];

	snprintf(buf, sizeof buf, "some-string \t\r\n\t\t\t    \r\n ");
	PT_ASSERT_STR_EQ(trim(buf), "some-string");

	snprintf(buf, sizeof buf, "\t\r\n\t\t\t    \r\n ");
	PT_ASSERT_STR_EQ(trim(buf), "");
    }
    PT_TEST(test_strtolower)
    {
	char buf[] = "MAKE me TO all LOWER";

	Strtolower(buf);
	PT_ASSERT_STR_EQ(buf, "make me to all lower");
    }
}

PT_SUITE(suite_wrapper)
{
    PT_TEST(test_xstrdup)
    {
	char *ref;

	ref = xstrdup("string to dup  --  return value must be freed");
	PT_ASSERT_STR_EQ(ref, "string to dup  --  return value must be freed");
	free(ref);
    }
    PT_TEST(test_strdup_printf)
    {
	char *ref;

	ref = Strdup_printf("%d %s", 31337, "test string");
	PT_ASSERT_STR_EQ(ref, "31337 test string");
	free(ref);
    }
}

int main(void)
{
    pt_add_suite(suite_various);
    pt_add_suite(suite_wrapper);
    return pt_run();
}
