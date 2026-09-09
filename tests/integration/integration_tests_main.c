#include "test_framework.h"
#include "integration/integration_tests.h"

TEST_INIT();

int main(void)
{
    printf("\n========================================\n");
    printf("   RUNNING ALL INTEGRATION TESTS\n");
    printf("========================================\n");

    run_macro_preprocessor_tests();
    run_assembler_pass1_tests();
    run_assembler_pass2_tests();
    run_output_writers_tests();
    run_end_to_end_tests();

    TEST_REPORT_SUMMARY();

    return TEST_EXIT_CODE();
}
