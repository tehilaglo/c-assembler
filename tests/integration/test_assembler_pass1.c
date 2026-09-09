#include "test_framework.h"
#include "test_helpers.h"
#include "assembler_pass1.h"

static void test_pass1_valid_program(void)
{
    const char *base_name = "tests/fixtures/pass1_valid";
    char as_path[512];

    const char *source =
        ".entry LOOP\n"
        ".extern EXTVAR\n"
        "MAIN: mov #5, r1\n"
        "LOOP: add r1, r2\n"
        "      lea S1, r3\n"
        "      jmp EXTVAR\n"
        "END:  hlt\n"
        "STR:  .string \"hello\"\n"
        "DATA: .data 10, -20\n"
        "S1:   .struct 5, \"xy\"\n";

    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", source);

    FILE *fp = fopen(as_path, "r");
    TEST_ASSERT_NOT_NULL(fp);

    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, base_name);

    bool result = assembler_pass1(fp, &asm_ctx);
    fclose(fp);

    TEST_ASSERT_TRUE(result);

    /* Check symbols */
    TEST_ASSERT_TRUE(symbol_exist(&asm_ctx.symbols, "MAIN"));
    TEST_ASSERT_TRUE(symbol_exist(&asm_ctx.symbols, "LOOP"));
    TEST_ASSERT_TRUE(symbol_exist(&asm_ctx.symbols, "END"));
    TEST_ASSERT_TRUE(symbol_exist(&asm_ctx.symbols, "STR"));
    TEST_ASSERT_TRUE(symbol_exist(&asm_ctx.symbols, "DATA"));
    TEST_ASSERT_TRUE(symbol_exist(&asm_ctx.symbols, "S1"));
    TEST_ASSERT_TRUE(symbol_exist(&asm_ctx.symbols, "EXTVAR"));

    Symbol *sym_main = get_symbol(&asm_ctx.symbols, "MAIN");
    TEST_ASSERT_NOT_NULL(sym_main);
    TEST_ASSERT_EQUAL_INT(1, sym_main->attr.code);
    TEST_ASSERT_EQUAL_INT(0, sym_main->address); /* first instruction at IC=0 */

    Symbol *sym_ext = get_symbol(&asm_ctx.symbols, "EXTVAR");
    TEST_ASSERT_NOT_NULL(sym_ext);
    TEST_ASSERT_EQUAL_INT(1, sym_ext->attr.external);

    Symbol *sym_str = get_symbol(&asm_ctx.symbols, "STR");
    TEST_ASSERT_NOT_NULL(sym_str);
    TEST_ASSERT_EQUAL_INT(1, sym_str->attr.data);
    TEST_ASSERT_EQUAL_INT(0, sym_str->address); /* first data at DC=0 */

    Symbol *sym_data = get_symbol(&asm_ctx.symbols, "DATA");
    TEST_ASSERT_NOT_NULL(sym_data);
    TEST_ASSERT_EQUAL_INT(1, sym_data->attr.data);
    TEST_ASSERT_EQUAL_INT(6, sym_data->address); /* after "hello\0" (6 words) */

    Symbol *sym_s1 = get_symbol(&asm_ctx.symbols, "S1");
    TEST_ASSERT_NOT_NULL(sym_s1);
    TEST_ASSERT_EQUAL_INT(1, sym_s1->attr.data);
    TEST_ASSERT_EQUAL_INT(8, sym_s1->address); /* after 6 + 2 = 8 words */

    /* Check DC: "hello\0" (6) + 10, -20 (2) + 5, "xy\0" (4) = 12 */
    TEST_ASSERT_EQUAL_INT(12, asm_ctx.dc);

    /* Check IC:
     * mov #5, r1 -> 3 words (0,1,2)
     * LOOP: add r1, r2 -> 2 words (3,4)
     * lea S1, r3 -> 3 words (5,6,7)
     * jmp EXT_VAR -> 2 words (8,9)
     * hlt -> 1 word (10)
     * total IC = 11
     */
    TEST_ASSERT_EQUAL_INT(11, asm_ctx.ic);

    cleanup_test_assembler(&asm_ctx);
    cleanup_assembler_artifacts(base_name);
}

static void test_pass1_invalid_programs(void)
{
    const char *base_name = "tests/fixtures/pass1_invalid";
    char as_path[512];

    /* Case 1: Unrecognized statement */
    const char *src1 = "foo bar baz\n";
    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", src1);
    FILE *fp = fopen(as_path, "r");
    TEST_ASSERT_NOT_NULL(fp);
    Assembler asm_ctx1;
    init_test_assembler(&asm_ctx1, base_name);
    TEST_ASSERT_FALSE(assembler_pass1(fp, &asm_ctx1));
    fclose(fp);
    cleanup_test_assembler(&asm_ctx1);
    cleanup_assembler_artifacts(base_name);

    /* Case 2: Duplicate label definition */
    const char *src2 =
        "LABEL: mov r1, r2\n"
        "LABEL: add r3, r4\n";
    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", src2);
    fp = fopen(as_path, "r");
    TEST_ASSERT_NOT_NULL(fp);
    Assembler asm_ctx2;
    init_test_assembler(&asm_ctx2, base_name);
    TEST_ASSERT_FALSE(assembler_pass1(fp, &asm_ctx2));
    fclose(fp);
    cleanup_test_assembler(&asm_ctx2);
    cleanup_assembler_artifacts(base_name);

    /* Case 3: Invalid operand */
    const char *src3 = "MAIN: mov #abc, r1\n";
    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", src3);
    fp = fopen(as_path, "r");
    TEST_ASSERT_NOT_NULL(fp);
    Assembler asm_ctx3;
    init_test_assembler(&asm_ctx3, base_name);
    TEST_ASSERT_FALSE(assembler_pass1(fp, &asm_ctx3));
    fclose(fp);
    cleanup_test_assembler(&asm_ctx3);
    cleanup_assembler_artifacts(base_name);

    /* Case 4: Label with no statement */
    const char *src4 = "EMPTY_LABEL:\n";
    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", src4);
    fp = fopen(as_path, "r");
    TEST_ASSERT_NOT_NULL(fp);
    Assembler asm_ctx4;
    init_test_assembler(&asm_ctx4, base_name);
    TEST_ASSERT_FALSE(assembler_pass1(fp, &asm_ctx4));
    fclose(fp);
    cleanup_test_assembler(&asm_ctx4);
    cleanup_assembler_artifacts(base_name);
}

void run_assembler_pass1_tests(void)
{
    TEST_SUITE_START("Assembler Pass 1 Integration");
    RUN_TEST(test_pass1_valid_program);
    RUN_TEST(test_pass1_invalid_programs);
}
