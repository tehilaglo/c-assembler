#include "test_framework.h"
#include "test_helpers.h"
#include "instruction_encoder.h"
#include "instr_operands_encoder.h"

static void test_zero_operand_instructions(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");

    char buf1[] = "";
    TEST_ASSERT_TRUE(encode_instruction(&asm_ctx, buf1, "rts"));
    TEST_ASSERT_EQUAL_INT(1, asm_ctx.ic);
    TEST_ASSERT_NOT_NULL(asm_ctx.code[0]);
    TEST_ASSERT_NOT_NULL(asm_ctx.code[0]->machine_code);
    TEST_ASSERT_EQUAL_INT(14, asm_ctx.code[0]->machine_code->opcode);
    TEST_ASSERT_EQUAL_INT(ABSOLUTE, asm_ctx.code[0]->machine_code->are);
    TEST_ASSERT_EQUAL_INT(0, asm_ctx.code[0]->machine_code->src_addr_mode);
    TEST_ASSERT_EQUAL_INT(0, asm_ctx.code[0]->machine_code->dst_addr_mode);

    char buf2[] = "";
    TEST_ASSERT_TRUE(encode_instruction(&asm_ctx, buf2, "hlt"));
    TEST_ASSERT_EQUAL_INT(2, asm_ctx.ic);
    TEST_ASSERT_EQUAL_INT(15, asm_ctx.code[1]->machine_code->opcode);

    cleanup_test_assembler(&asm_ctx);
}

static void test_single_operand_instructions(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");

    /* prn with immediate: prn #5 */
    char buf1[] = "#5";
    TEST_ASSERT_TRUE(encode_instruction(&asm_ctx, buf1, "prn"));
    TEST_ASSERT_EQUAL_INT(2, asm_ctx.ic);
    TEST_ASSERT_EQUAL_INT(12, asm_ctx.code[0]->machine_code->opcode);
    TEST_ASSERT_EQUAL_INT(IMMDT_ADDRESS, asm_ctx.code[0]->machine_code->dst_addr_mode);
    TEST_ASSERT_EQUAL_INT(0, asm_ctx.code[0]->machine_code->src_addr_mode);
    TEST_ASSERT_NOT_NULL(asm_ctx.code[1]->machine_code);
    TEST_ASSERT_EQUAL_INT(ABSOLUTE, asm_ctx.code[1]->machine_code->are);

    /* not with register: not r3 */
    char buf2[] = "r3";
    TEST_ASSERT_TRUE(encode_instruction(&asm_ctx, buf2, "not"));
    TEST_ASSERT_EQUAL_INT(4, asm_ctx.ic); /* +2 words (opcode word + reg word) */
    TEST_ASSERT_EQUAL_INT(4, asm_ctx.code[2]->machine_code->opcode);
    TEST_ASSERT_EQUAL_INT(REG_ADDRESS, asm_ctx.code[2]->machine_code->dst_addr_mode);
    TEST_ASSERT_NOT_NULL(asm_ctx.code[3]->machine_code);

    /* jmp with direct label: jmp LOOP */
    char buf3[] = "LOOP";
    TEST_ASSERT_TRUE(encode_instruction(&asm_ctx, buf3, "jmp"));
    TEST_ASSERT_EQUAL_INT(6, asm_ctx.ic); /* +2 words (opcode word + label placeholder) */
    TEST_ASSERT_EQUAL_INT(9, asm_ctx.code[4]->machine_code->opcode);
    TEST_ASSERT_EQUAL_INT(DIRECT_ADDRESS, asm_ctx.code[4]->machine_code->dst_addr_mode);
    TEST_ASSERT_EQUAL_STR("LOOP", asm_ctx.code[5]->symbol_name);

    /* inc with struct: inc S1.2 */
    char buf4[] = "S1.2";
    TEST_ASSERT_TRUE(encode_instruction(&asm_ctx, buf4, "inc"));
    TEST_ASSERT_EQUAL_INT(9, asm_ctx.ic); /* +3 words (opcode word + symbol + field) */
    TEST_ASSERT_EQUAL_INT(7, asm_ctx.code[6]->machine_code->opcode);
    TEST_ASSERT_EQUAL_INT(STRUCT_ADDRESS, asm_ctx.code[6]->machine_code->dst_addr_mode);
    TEST_ASSERT_EQUAL_STR("S1", asm_ctx.code[7]->symbol_name);
    TEST_ASSERT_NOT_NULL(asm_ctx.code[8]->machine_code);
    TEST_ASSERT_EQUAL_INT(2, asm_ctx.code[8]->machine_code->dst_addr_mode);

    cleanup_test_assembler(&asm_ctx);
}

static void test_two_operand_instructions(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");

    /* mov immediate to register: mov #5, r1 */
    char buf1[] = "#5, r1";
    TEST_ASSERT_TRUE(encode_instruction(&asm_ctx, buf1, "mov"));
    TEST_ASSERT_EQUAL_INT(3, asm_ctx.ic); /* opcode word + immed word + reg word */
    TEST_ASSERT_EQUAL_INT(0, asm_ctx.code[0]->machine_code->opcode);
    TEST_ASSERT_EQUAL_INT(IMMDT_ADDRESS, asm_ctx.code[0]->machine_code->src_addr_mode);
    TEST_ASSERT_EQUAL_INT(REG_ADDRESS, asm_ctx.code[0]->machine_code->dst_addr_mode);

    /* two-register optimization: mov r1, r2 (shared register word!) */
    char buf2[] = "r1, r2";
    TEST_ASSERT_TRUE(encode_instruction(&asm_ctx, buf2, "mov"));
    TEST_ASSERT_EQUAL_INT(5, asm_ctx.ic); /* exactly 2 words: opcode word + 1 shared reg word */
    TEST_ASSERT_EQUAL_INT(0, asm_ctx.code[3]->machine_code->opcode);
    TEST_ASSERT_EQUAL_INT(REG_ADDRESS, asm_ctx.code[3]->machine_code->src_addr_mode);
    TEST_ASSERT_EQUAL_INT(REG_ADDRESS, asm_ctx.code[3]->machine_code->dst_addr_mode);
    TEST_ASSERT_NOT_NULL(asm_ctx.code[4]->machine_code);

    /* lea direct to register: lea S1, r2 */
    char buf3[] = "S1, r2";
    TEST_ASSERT_TRUE(encode_instruction(&asm_ctx, buf3, "lea"));
    TEST_ASSERT_EQUAL_INT(8, asm_ctx.ic); /* opcode word + symbol + reg word */
    TEST_ASSERT_EQUAL_INT(6, asm_ctx.code[5]->machine_code->opcode);
    TEST_ASSERT_EQUAL_INT(DIRECT_ADDRESS, asm_ctx.code[5]->machine_code->src_addr_mode);
    TEST_ASSERT_EQUAL_INT(REG_ADDRESS, asm_ctx.code[5]->machine_code->dst_addr_mode);
    TEST_ASSERT_EQUAL_STR("S1", asm_ctx.code[6]->symbol_name);

    /* sub struct to direct: sub S1.1, W */
    char buf4[] = "S1.1, W";
    TEST_ASSERT_TRUE(encode_instruction(&asm_ctx, buf4, "sub"));
    TEST_ASSERT_EQUAL_INT(12, asm_ctx.ic); /* opcode word + struct symbol + struct field + direct symbol */
    TEST_ASSERT_EQUAL_INT(3, asm_ctx.code[8]->machine_code->opcode);
    TEST_ASSERT_EQUAL_INT(STRUCT_ADDRESS, asm_ctx.code[8]->machine_code->src_addr_mode);
    TEST_ASSERT_EQUAL_INT(DIRECT_ADDRESS, asm_ctx.code[8]->machine_code->dst_addr_mode);
    TEST_ASSERT_EQUAL_STR("S1", asm_ctx.code[9]->symbol_name);
    TEST_ASSERT_EQUAL_INT(1, asm_ctx.code[10]->machine_code->dst_addr_mode);
    TEST_ASSERT_EQUAL_STR("W", asm_ctx.code[11]->symbol_name);

    cleanup_test_assembler(&asm_ctx);
}

static void test_instruction_encoder_invalid_cases(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");

    /* Invalid instruction mnemonic */
    char buf1[] = "r1, r2";
    TEST_ASSERT_FALSE(encode_instruction(&asm_ctx, buf1, "invalid_instr"));

    /* Missing operands */
    char buf2[] = "";
    TEST_ASSERT_FALSE(encode_instruction(&asm_ctx, buf2, "mov"));

    char buf3[] = "r1";
    TEST_ASSERT_FALSE(encode_instruction(&asm_ctx, buf3, "mov"));

    /* Invalid addressing mode for lea (immediate source not allowed) */
    char buf4[] = "#5, r1";
    TEST_ASSERT_FALSE(encode_instruction(&asm_ctx, buf4, "lea"));

    /* Too many operands */
    char buf5[] = "r1, r2, r3";
    TEST_ASSERT_FALSE(encode_instruction(&asm_ctx, buf5, "mov"));

    char buf6[] = "extra";
    TEST_ASSERT_TRUE(encode_instruction(&asm_ctx, buf6, "rts")); /* logs error internally */

    /* NULL handling */
    TEST_ASSERT_FALSE(encode_instruction(NULL, buf1, "mov"));
    TEST_ASSERT_FALSE(encode_instruction(&asm_ctx, NULL, "mov"));
    TEST_ASSERT_FALSE(encode_instruction(&asm_ctx, buf1, NULL));

    cleanup_test_assembler(&asm_ctx);
}

void run_instruction_encoder_tests(void)
{
    TEST_SUITE_START("Instruction Encoder");
    RUN_TEST(test_zero_operand_instructions);
    RUN_TEST(test_single_operand_instructions);
    RUN_TEST(test_two_operand_instructions);
    RUN_TEST(test_instruction_encoder_invalid_cases);
}
