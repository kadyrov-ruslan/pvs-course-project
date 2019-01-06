#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
#include <stdio.h>

/* Test Suite setup and cleanup functions: */
int init_suite(void) { return 0; }
int clean_suite(void) { return 0; }

void test_case_sample(void)
{
    CU_ASSERT(CU_TRUE);
    CU_ASSERT_NOT_EQUAL(2, -1);
    CU_ASSERT_STRING_EQUAL("string #1", "string #1");
    CU_ASSERT_STRING_NOT_EQUAL("string #1", "string #2");

    CU_ASSERT(CU_FALSE);
    CU_ASSERT_EQUAL(2, 3);
    CU_ASSERT_STRING_NOT_EQUAL("string #1", "string #1");
    CU_ASSERT_STRING_EQUAL("string #1", "string #2");
}

void test_1(void)
{
    CU_ASSERT_EQUAL(2, 2);
    CU_ASSERT_EQUAL(5, 5);
}

void test_2(void)
{
    CU_ASSERT_EQUAL(2, 2);
    CU_ASSERT_NOT_EQUAL(-5, 0);
    CU_ASSERT_EQUAL(-1, -1);
}

void test_3(void)
{
    CU_ASSERT_EQUAL(-1, -1);
}

int main(void)
{
    CU_pSuite pSuite = NULL;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* add a suite to the registry */
    pSuite = CU_add_suite("smtp_client_test_suite", init_suite, clean_suite);
    if (NULL == pSuite)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "test_1", test_1)) ||
        (NULL == CU_add_test(pSuite, "test_2", test_2)) ||
        (NULL == CU_add_test(pSuite, "test_3", test_3)))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Run all tests using the basic interface
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    printf("\n");
    CU_basic_show_failures(CU_get_failure_list());
    printf("\n\n");
    /*
   // Run all tests using the automated interface
   CU_automated_run_tests();
   CU_list_tests_to_file();

   // Run all tests using the console interface
   CU_console_run_tests();
*/
    /* Clean up registry and return */
    CU_cleanup_registry();
    return CU_get_error();
}
