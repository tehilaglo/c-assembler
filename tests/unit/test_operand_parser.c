#include "test_framework.h"
#include "test_helpers.h"
#include "operand_parser.h"
#include "macro_table.h"

static void test_immediate_operand_valid(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");
    int num = 0;

    char op1[] = "#5";
    TEST_ASSERT_EQUAL_INT(OPERAND_OK, parse_immediate_operand(&asm_ctx, op1, "mov", &num));
    TEST_ASSERT_EQUAL_INT(5, num);

    char op2[] = "#-5";
    TEST_ASSERT_EQUAL_INT(OPERAND_OK, parse_immediate_operand(&asm_ctx, op2, "mov", &num));
    TEST_ASSERT_EQUAL_INT(-5, num);

    char op3[] = "#+7";
    TEST_ASSERT_EQUAL_INT(OPERAND_OK, parse_immediate_operand(&asm_ctx, op3, "mov", &num));
    TEST_ASSERT_EQUAL_INT(7, num);

    char op4[] = "#0";
    TEST_ASSERT_EQUAL_INT(OPERAND_OK, parse_immediate_operand(&asm_ctx, op4, "mov", &num));
    TEST_ASSERT_EQUAL_INT(0, num);

    char op5[] = "#-0";
    TEST_ASSERT_EQUAL_INT(OPERAND_OK, parse_immediate_operand(&asm_ctx, op5, "mov", &num));
    TEST_ASSERT_EQUAL_INT(0, num);

    char op6[] = "#+0";
    TEST_ASSERT_EQUAL_INT(OPERAND_OK, parse_immediate_operand(&asm_ctx, op6, "mov", &num));
    TEST_ASSERT_EQUAL_INT(0, num);

    char op7[] = "   \t #42 \t ";
    TEST_ASSERT_EQUAL_INT(OPERAND_OK, parse_immediate_operand(&asm_ctx, op7, "mov", &num));
    TEST_ASSERT_EQUAL_INT(42, num);

    cleanup_test_assembler(&asm_ctx);
}

static void test_immediate_operand_invalid_and_mismatch(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");
    int num = 0;

    /* Not an immediate operand */
    char op_not_match1[] = "r1";
    TEST_ASSERT_EQUAL_INT(OPERAND_NOT_MATCH, parse_immediate_operand(&asm_ctx, op_not_match1, "mov", &num));

    char op_not_match2[] = "LABEL";
    TEST_ASSERT_EQUAL_INT(OPERAND_NOT_MATCH, parse_immediate_operand(&asm_ctx, op_not_match2, "mov", &num));

    /* Missing number after # */
    char op_err1[] = "#";
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_immediate_operand(&asm_ctx, op_err1, "mov", &num));

    char op_err2[] = "#-";
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_immediate_operand(&asm_ctx, op_err2, "mov", &num));

    char op_err3[] = "#+";
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_immediate_operand(&asm_ctx, op_err3, "mov", &num));

    /* Invalid characters */
    char op_err4[] = "#abc";
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_immediate_operand(&asm_ctx, op_err4, "mov", &num));

    char op_err5[] = "#5xyz";
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_immediate_operand(&asm_ctx, op_err5, "mov", &num));

    /* NULL parameters */
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_immediate_operand(NULL, op_err1, "mov", &num));
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_immediate_operand(&asm_ctx, NULL, "mov", &num));
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_immediate_operand(&asm_ctx, op_err1, NULL, &num));
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_immediate_operand(&asm_ctx, op_err1, "mov", NULL));

    cleanup_test_assembler(&asm_ctx);
}

static void test_register_operand_valid(void)
{
    int reg_num = -1;

    char r0[] = "r0";
    TEST_ASSERT_EQUAL_INT(OPERAND_OK, parse_register_operand(r0, &reg_num));
    TEST_ASSERT_EQUAL_INT(0, reg_num);

    char r7[] = "r7";
    TEST_ASSERT_EQUAL_INT(OPERAND_OK, parse_register_operand(r7, &reg_num));
    TEST_ASSERT_EQUAL_INT(7, reg_num);

    char r3_ws[] = "  \t r3 \t ";
    TEST_ASSERT_EQUAL_INT(OPERAND_OK, parse_register_operand(r3_ws, &reg_num));
    TEST_ASSERT_EQUAL_INT(3, reg_num);
}

static void test_register_operand_invalid_and_mismatch(void)
{
    int reg_num = -1;

    char inv1[] = "r8";
    TEST_ASSERT_EQUAL_INT(OPERAND_NOT_MATCH, parse_register_operand(inv1, &reg_num));

    char inv2[] = "r-1";
    TEST_ASSERT_EQUAL_INT(OPERAND_NOT_MATCH, parse_register_operand(inv2, &reg_num));

    char inv3[] = "r";
    TEST_ASSERT_EQUAL_INT(OPERAND_NOT_MATCH, parse_register_operand(inv3, &reg_num));

    char inv4[] = "reg1";
    TEST_ASSERT_EQUAL_INT(OPERAND_NOT_MATCH, parse_register_operand(inv4, &reg_num));

    char inv5[] = "#5";
    TEST_ASSERT_EQUAL_INT(OPERAND_NOT_MATCH, parse_register_operand(inv5, &reg_num));

    char inv6[] = "LABEL";
    TEST_ASSERT_EQUAL_INT(OPERAND_NOT_MATCH, parse_register_operand(inv6, &reg_num));

    /* NULL handling */
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_register_operand(NULL, &reg_num));
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_register_operand(inv1, NULL));
}

static void test_direct_operand_valid(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");
    char symbol[MAX_LINE_LEN];

    char op1[] = "LOOP";
    TEST_ASSERT_EQUAL_INT(OPERAND_OK, parse_direct_operand(&asm_ctx, op1, "jmp", symbol));
    TEST_ASSERT_EQUAL_STR("LOOP", symbol);

    char op2[] = "STR1";
    TEST_ASSERT_EQUAL_INT(OPERAND_OK, parse_direct_operand(&asm_ctx, op2, "add", symbol));
    TEST_ASSERT_EQUAL_STR("STR1", symbol);

    cleanup_test_assembler(&asm_ctx);
}

static void test_direct_operand_invalid(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");
    char symbol[MAX_LINE_LEN];

    /* Missing operand */
    char empty1[] = "";
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_direct_operand(&asm_ctx, empty1, "jmp", symbol));

    char empty2[] = "   \t \n";
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_direct_operand(&asm_ctx, empty2, "jmp", symbol));

    /* Illegal characters */
    char inv_char1[] = "FOO-BAR";
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_direct_operand(&asm_ctx, inv_char1, "jmp", symbol));

    char inv_char2[] = "FOO$BAR";
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_direct_operand(&asm_ctx, inv_char2, "jmp", symbol));

    /* Reserved words */
    char res_dir[] = "data";
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_direct_operand(&asm_ctx, res_dir, "jmp", symbol));

    char res_instr[] = "mov";
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_direct_operand(&asm_ctx, res_instr, "jmp", symbol));

    char res_reg[] = "r0";
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_direct_operand(&asm_ctx, res_reg, "jmp", symbol));

    /* NULL parameters */
    char valid_op[] = "LABEL";
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_direct_operand(NULL, valid_op, "jmp", symbol));
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_direct_operand(&asm_ctx, NULL, "jmp", symbol));
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_direct_operand(&asm_ctx, valid_op, NULL, symbol));
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_direct_operand(&asm_ctx, valid_op, "jmp", NULL));

    cleanup_test_assembler(&asm_ctx);
}

static void test_struct_operand_valid(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");
    char symbol[MAX_LINE_LEN];
    int field = 0;

    char op1[] = "S1.1";
    TEST_ASSERT_EQUAL_INT(OPERAND_OK, parse_struct_operand(&asm_ctx, op1, "mov", symbol, &field));
    TEST_ASSERT_EQUAL_STR("S1", symbol);
    TEST_ASSERT_EQUAL_INT(1, field);

    char op2[] = "  \t MyStruct.2 \t ";
    TEST_ASSERT_EQUAL_INT(OPERAND_OK, parse_struct_operand(&asm_ctx, op2, "mov", symbol, &field));
    TEST_ASSERT_EQUAL_STR("MyStruct", symbol);
    TEST_ASSERT_EQUAL_INT(2, field);

    cleanup_test_assembler(&asm_ctx);
}

static void test_struct_operand_invalid_and_mismatch(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");
    char symbol[MAX_LINE_LEN];
    int field = 0;

    /* No dot -> OPERAND_NOT_MATCH */
    char no_dot[] = "S1";
    TEST_ASSERT_EQUAL_INT(OPERAND_NOT_MATCH, parse_struct_operand(&asm_ctx, no_dot, "mov", symbol, &field));

    /* Missing field */
    char missing_field[] = "S1.";
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_struct_operand(&asm_ctx, missing_field, "mov", symbol, &field));

    /* Invalid field numbers */
    char inv_field1[] = "S1.0";
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_struct_operand(&asm_ctx, inv_field1, "mov", symbol, &field));

    char inv_field2[] = "S1.3";
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_struct_operand(&asm_ctx, inv_field2, "mov", symbol, &field));

    char inv_field3[] = "S1.a";
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_struct_operand(&asm_ctx, inv_field3, "mov", symbol, &field));

    /* NULL parameters */
    char valid_st[] = "S1.1";
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_struct_operand(NULL, valid_st, "mov", symbol, &field));
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_struct_operand(&asm_ctx, NULL, "mov", symbol, &field));
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_struct_operand(&asm_ctx, valid_st, NULL, symbol, &field));
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_struct_operand(&asm_ctx, valid_st, "mov", NULL, &field));
    TEST_ASSERT_EQUAL_INT(OPERAND_ERR, parse_struct_operand(&asm_ctx, valid_st, "mov", symbol, NULL));

    cleanup_test_assembler(&asm_ctx);
}

void run_operand_parser_tests(void)
{
    TEST_SUITE_START("Operand Parser");
    RUN_TEST(test_immediate_operand_valid);
    RUN_TEST(test_immediate_operand_invalid_and_mismatch);
    RUN_TEST(test_register_operand_valid);
    RUN_TEST(test_register_operand_invalid_and_mismatch);
    RUN_TEST(test_direct_operand_valid);
    RUN_TEST(test_direct_operand_invalid);
    RUN_TEST(test_struct_operand_valid);
    RUN_TEST(test_struct_operand_invalid_and_mismatch);
}
