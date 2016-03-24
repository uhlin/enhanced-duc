#include <stdio.h>

#include "ptest.h"

#include "various.h"

PT_SUITE(suite_various)
{
    PT_TEST(test_size_product)
    {
	PT_ASSERT(size_product(3, 3) == 9);
	PT_ASSERT(size_product(5, 20) == 100);
	PT_ASSERT(size_product(0, 1000) == 0);
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

int main(void)
{
    pt_add_suite(suite_various);
    return pt_run();
}
