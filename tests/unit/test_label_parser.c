#include "test_framework.h"
#include "test_helpers.h"
#include "label_parser.h"
#include "symbol_table.h"
#include "macro_table.h"

static void test_valid_labels(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");
    char symbol[MAX_LINE_LEN];

    TEST_ASSERT_EQUAL_INT(LABEL_VALID, parse_label_definition(&asm_ctx, "MAIN:", symbol));
    TEST_ASSERT_EQUAL_STR("MAIN", symbol);

    TEST_ASSERT_EQUAL_INT(LABEL_VALID, parse_label_definition(&asm_ctx, "Loop1:", symbol));
    TEST_ASSERT_EQUAL_STR("Loop1", symbol);

    TEST_ASSERT_EQUAL_INT(LABEL_VALID, parse_label_definition(&asm_ctx, "x:", symbol));
    TEST_ASSERT_EQUAL_STR("x", symbol);

    cleanup_test_assembler(&asm_ctx);
}

static void test_label_length_boundaries(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");
    char symbol[MAX_LINE_LEN];

    /* Exactly 30 characters + ':' */
    const char *label_30 = "abcdefghijklmnopqrstuvwxyz1234:";
    TEST_ASSERT_EQUAL_INT(31, strlen(label_30)); /* 30 chars + ':' */
    TEST_ASSERT_EQUAL_INT(LABEL_VALID, parse_label_definition(&asm_ctx, label_30, symbol));
    TEST_ASSERT_EQUAL_STR("abcdefghijklmnopqrstuvwxyz1234", symbol);

    /* 31 characters + ':' -> exceeds MAX_LABEL_LEN */
    const char *label_31 = "abcdefghijklmnopqrstuvwxyz12345:";
    TEST_ASSERT_EQUAL_INT(32, strlen(label_31)); /* 31 chars + ':' */
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, label_31, symbol));

    cleanup_test_assembler(&asm_ctx);
}

static void test_invalid_label_syntax(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");
    char symbol[MAX_LINE_LEN];

    /* Starts with digit */
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, "1LABEL:", symbol));

    /* Illegal characters */
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, "MY-VAR:", symbol));
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, "MY_VAR:", symbol));
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, "VAR$:", symbol));
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, "VAR#:", symbol));
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, "VAR.:", symbol));

    cleanup_test_assembler(&asm_ctx);
}

static void test_reserved_words_as_labels(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");
    char symbol[MAX_LINE_LEN];

    /* Instruction names */
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, "mov:", symbol));
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, "cmp:", symbol));
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, "add:", symbol));
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, "hlt:", symbol));

    /* Register names */
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, "r0:", symbol));
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, "r7:", symbol));

    /* Directive names */
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, "data:", symbol));
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, "string:", symbol));
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, "struct:", symbol));
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, "entry:", symbol));
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, "extern:", symbol));

    cleanup_test_assembler(&asm_ctx);
}

static void test_label_not_present_and_malformed(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");
    char symbol[MAX_LINE_LEN];

    /* No trailing colon */
    TEST_ASSERT_EQUAL_INT(LABEL_NOT_PRESENT, parse_label_definition(&asm_ctx, "MAIN", symbol));
    TEST_ASSERT_EQUAL_INT(LABEL_NOT_PRESENT, parse_label_definition(&asm_ctx, "mov", symbol));
    TEST_ASSERT_EQUAL_INT(LABEL_NOT_PRESENT, parse_label_definition(&asm_ctx, ".data", symbol));

    /* Just a colon or empty */
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, ":", symbol));
    TEST_ASSERT_EQUAL_INT(LABEL_NOT_PRESENT, parse_label_definition(&asm_ctx, "", symbol));

    /* NULL parameters */
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(NULL, "MAIN:", symbol));
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, NULL, symbol));
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, "MAIN:", NULL));

    cleanup_test_assembler(&asm_ctx);
}

static void test_duplicate_label_in_symbol_table(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");
    char symbol[MAX_LINE_LEN];

    /* First definition succeeds */
    TEST_ASSERT_EQUAL_INT(LABEL_VALID, parse_label_definition(&asm_ctx, "LOOP:", symbol));

    /* Insert it into the symbol table */
    Symbol *node = new_symbol("LOOP", SYMBOL_CODE, 100);
    insert_symbol(&asm_ctx.symbols, node);

    /* Second definition with same name should return LABEL_INVALID */
    TEST_ASSERT_EQUAL_INT(LABEL_INVALID, parse_label_definition(&asm_ctx, "LOOP:", symbol));

    /* Different label is still valid */
    TEST_ASSERT_EQUAL_INT(LABEL_VALID, parse_label_definition(&asm_ctx, "OTHER:", symbol));

    cleanup_test_assembler(&asm_ctx);
}

void run_label_parser_tests(void)
{
    TEST_SUITE_START("Label Parser");
    RUN_TEST(test_valid_labels);
    RUN_TEST(test_label_length_boundaries);
    RUN_TEST(test_invalid_label_syntax);
    RUN_TEST(test_reserved_words_as_labels);
    RUN_TEST(test_label_not_present_and_malformed);
    RUN_TEST(test_duplicate_label_in_symbol_table);
}
