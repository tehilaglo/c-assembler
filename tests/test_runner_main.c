#include "test_framework.h"
#include "unit/unit_tests.h"
#include "integration/integration_tests.h"

TEST_INIT();

int main(void)
{
    printf("\n========================================\n");
    printf("   RUNNING ALL ASSEMBLER TEST SUITES\n");
    printf("========================================\n");

    /* Unit Tests */
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

    /* Integration Tests */
    run_macro_preprocessor_tests();
    run_assembler_pass1_tests();
    run_assembler_pass2_tests();
    run_output_writers_tests();
    run_end_to_end_tests();

    TEST_REPORT_SUMMARY();

    return TEST_EXIT_CODE();
}
