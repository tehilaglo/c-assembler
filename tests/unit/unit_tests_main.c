#include "test_framework.h"
#include "unit/unit_tests.h"

TEST_INIT();

int main(void)
{
    printf("\n========================================\n");
    printf("   RUNNING ALL UNIT TESTS\n");
    printf("========================================\n");

    run_isa_token_tests();
    run_opcode_rules_tests();
    run_line_parser_tests();
    run_label_parser_tests();
    run_operand_parser_tests();
    run_symbol_table_tests();
    run_macro_table_tests();
    run_data_encoder_tests();
    run_instruction_encoder_tests();
    run_input_validator_tests();

    TEST_REPORT_SUMMARY();

    return TEST_EXIT_CODE();
}
