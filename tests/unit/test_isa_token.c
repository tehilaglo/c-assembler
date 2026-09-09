#include "test_framework.h"
#include "isa.h"
#include "token_classifier.h"

static void test_all_valid_instructions(void)
{
    const char *valid_instrs[] = {
        "mov", "cmp", "add", "sub",
        "not", "clr", "lea", "inc",
        "dec", "jmp", "bne", "get",
        "prn", "jsr", "rts", "hlt"
    };

    for (int i = 0; i < 16; ++i)
    {
        TEST_ASSERT_TRUE(is_instruction(valid_instrs[i]));
        TEST_ASSERT_EQUAL_INT(i, isa_get_opcode(valid_instrs[i]));
    }
}

static void test_all_valid_registers(void)
{
    const char *valid_regs[] = {
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
    };

    for (int i = 0; i < 8; ++i)
    {
        TEST_ASSERT_TRUE(is_register(valid_regs[i]));
    }
}

static void test_invalid_instructions_and_registers(void)
{
    const char *invalid_instrs[] = {
        "move", "jmp1", "hlt0", "addd", "subb", "noop", "push", "pop", "123", "", "MOV", "HLT"
    };

    for (size_t i = 0; i < sizeof(invalid_instrs) / sizeof(invalid_instrs[0]); ++i)
    {
        TEST_ASSERT_FALSE(is_instruction(invalid_instrs[i]));
    }

    TEST_ASSERT_FALSE(is_instruction(NULL));

    const char *invalid_regs[] = {
        "r8", "r-1", "r01", "reg", "R0", "r", "r9", "r10", "x", ""
    };

    for (size_t i = 0; i < sizeof(invalid_regs) / sizeof(invalid_regs[0]); ++i)
    {
        TEST_ASSERT_FALSE(is_register(invalid_regs[i]));
    }

    TEST_ASSERT_FALSE(is_register(NULL));
}

static void test_all_directives(void)
{
    TEST_ASSERT_TRUE(is_data(".data"));
    TEST_ASSERT_FALSE(is_data(".string"));
    TEST_ASSERT_FALSE(is_data("data"));
    TEST_ASSERT_FALSE(is_data(NULL));

    TEST_ASSERT_TRUE(is_string(".string"));
    TEST_ASSERT_FALSE(is_string(".data"));
    TEST_ASSERT_FALSE(is_string("string"));
    TEST_ASSERT_FALSE(is_string(NULL));

    TEST_ASSERT_TRUE(is_struct(".struct"));
    TEST_ASSERT_FALSE(is_struct(".entry"));
    TEST_ASSERT_FALSE(is_struct("struct"));
    TEST_ASSERT_FALSE(is_struct(NULL));

    TEST_ASSERT_TRUE(is_entry(".entry"));
    TEST_ASSERT_FALSE(is_entry(".extern"));
    TEST_ASSERT_FALSE(is_entry("entry"));
    TEST_ASSERT_FALSE(is_entry(NULL));

    TEST_ASSERT_TRUE(is_extern(".extern"));
    TEST_ASSERT_FALSE(is_extern(".data"));
    TEST_ASSERT_FALSE(is_extern("extern"));
    TEST_ASSERT_FALSE(is_extern(NULL));
}

static void test_word_is_directive(void)
{
    TEST_ASSERT_TRUE(word_is_directive("data"));
    TEST_ASSERT_TRUE(word_is_directive("string"));
    TEST_ASSERT_TRUE(word_is_directive("struct"));
    TEST_ASSERT_TRUE(word_is_directive("entry"));
    TEST_ASSERT_TRUE(word_is_directive("extern"));

    TEST_ASSERT_FALSE(word_is_directive("mov"));
    TEST_ASSERT_FALSE(word_is_directive("r1"));
    TEST_ASSERT_FALSE(word_is_directive("foo"));
    TEST_ASSERT_FALSE(word_is_directive(".data"));
    TEST_ASSERT_FALSE(word_is_directive(""));
    TEST_ASSERT_FALSE(word_is_directive(NULL));
}

static void test_opcode_lookup_and_invalid(void)
{
    TEST_ASSERT_EQUAL_INT(0, isa_get_opcode("mov"));
    TEST_ASSERT_EQUAL_INT(1, isa_get_opcode("cmp"));
    TEST_ASSERT_EQUAL_INT(2, isa_get_opcode("add"));
    TEST_ASSERT_EQUAL_INT(3, isa_get_opcode("sub"));
    TEST_ASSERT_EQUAL_INT(4, isa_get_opcode("not"));
    TEST_ASSERT_EQUAL_INT(5, isa_get_opcode("clr"));
    TEST_ASSERT_EQUAL_INT(6, isa_get_opcode("lea"));
    TEST_ASSERT_EQUAL_INT(7, isa_get_opcode("inc"));
    TEST_ASSERT_EQUAL_INT(8, isa_get_opcode("dec"));
    TEST_ASSERT_EQUAL_INT(9, isa_get_opcode("jmp"));
    TEST_ASSERT_EQUAL_INT(10, isa_get_opcode("bne"));
    TEST_ASSERT_EQUAL_INT(11, isa_get_opcode("get"));
    TEST_ASSERT_EQUAL_INT(12, isa_get_opcode("prn"));
    TEST_ASSERT_EQUAL_INT(13, isa_get_opcode("jsr"));
    TEST_ASSERT_EQUAL_INT(14, isa_get_opcode("rts"));
    TEST_ASSERT_EQUAL_INT(15, isa_get_opcode("hlt"));

    TEST_ASSERT_EQUAL_INT((uint8_t)-1, isa_get_opcode("invalid"));
    TEST_ASSERT_EQUAL_INT((uint8_t)-1, isa_get_opcode(""));
    TEST_ASSERT_EQUAL_INT((uint8_t)-1, isa_get_opcode(NULL));
}

static void test_base32_symbol_lookup(void)
{
    const char expected_b32[] = {
        '!', '@', '#', '$', '%', '^', '&', '*',
        '<', '>', 'a', 'b', 'c', 'd', 'e', 'f',
        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v'
    };

    for (int i = 0; i < 32; ++i)
    {
        TEST_ASSERT_EQUAL_CHAR(expected_b32[i], get_b32_symbol((char)i));
    }

    TEST_ASSERT_EQUAL_CHAR(-1, get_b32_symbol(-1));
    TEST_ASSERT_EQUAL_CHAR(-1, get_b32_symbol(32));
    TEST_ASSERT_EQUAL_CHAR(-1, get_b32_symbol(100));
}

static void test_instruction_and_register_boundaries(void)
{
    TEST_ASSERT_EQUAL_STR("mov", isa_get_instr_name(0));
    TEST_ASSERT_EQUAL_STR("hlt", isa_get_instr_name(15));
    TEST_ASSERT_NULL(isa_get_instr_name(-1));
    TEST_ASSERT_NULL(isa_get_instr_name(16));
    TEST_ASSERT_NULL(isa_get_instr_name(100));

    TEST_ASSERT_EQUAL_STR("r0", isa_get_register(0));
    TEST_ASSERT_EQUAL_STR("r7", isa_get_register(7));
    TEST_ASSERT_NULL(isa_get_register(-1));
    TEST_ASSERT_NULL(isa_get_register(8));
    TEST_ASSERT_NULL(isa_get_register(100));
}

void run_isa_token_tests(void)
{
    TEST_SUITE_START("ISA & Token Classification");
    RUN_TEST(test_all_valid_instructions);
    RUN_TEST(test_all_valid_registers);
    RUN_TEST(test_invalid_instructions_and_registers);
    RUN_TEST(test_all_directives);
    RUN_TEST(test_word_is_directive);
    RUN_TEST(test_opcode_lookup_and_invalid);
    RUN_TEST(test_base32_symbol_lookup);
    RUN_TEST(test_instruction_and_register_boundaries);
}
