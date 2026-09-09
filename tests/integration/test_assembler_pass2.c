#include "test_framework.h"
#include "test_helpers.h"
#include "assembler_pass1.h"
#include "assembler_pass2.h"
#include "file_config.h"

static void test_pass2_symbol_resolution_and_file_creation(void)
{
    const char *base_name = "tests/fixtures/pass2_valid";
    char as_path[512];
    char ob_path[512];
    char ent_path[512];
    char ext_path[512];

    snprintf(ob_path, sizeof(ob_path), "%s.ob", base_name);
    snprintf(ent_path, sizeof(ent_path), "%s.ent", base_name);
    snprintf(ext_path, sizeof(ext_path), "%s.ext", base_name);

    const char *source =
        ".entry LOOP\n"
        ".entry LENGTH\n"
        ".extern EXTL3\n"
        "MAIN:   mov S1.1, EXTL3\n"
        "        add r2, STR\n"
        "LOOP:   jmp EXTL3\n"
        "        inc K\n"
        "END:    hlt\n"
        "STR:    .string \"abcdef\"\n"
        "LENGTH: .data 6, -9, 15\n"
        "K:      .data 22\n"
        "S1:     .struct 8, \"ab\"\n";

    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", source);

    FILE *fp = fopen(as_path, "r");
    TEST_ASSERT_NOT_NULL(fp);

    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, base_name);

    bool p1_ok = assembler_pass1(fp, &asm_ctx);
    TEST_ASSERT_TRUE(p1_ok);

    /* Rewind fp before pass 2 */
    fseek(fp, 0, SEEK_SET);

    assembler_pass2(fp, &asm_ctx);
    fclose(fp);

    /* Verify output files were created */
    TEST_ASSERT_TRUE(file_exists(ob_path));
    TEST_ASSERT_TRUE(file_exists(ent_path));
    TEST_ASSERT_TRUE(file_exists(ext_path));

    /* Verify resolved symbols */
    Symbol *sym_loop = get_symbol(&asm_ctx.symbols, "LOOP");
    TEST_ASSERT_NOT_NULL(sym_loop);
    TEST_ASSERT_EQUAL_INT(1, sym_loop->attr.entry);

    Symbol *sym_length = get_symbol(&asm_ctx.symbols, "LENGTH");
    TEST_ASSERT_NOT_NULL(sym_length);
    TEST_ASSERT_EQUAL_INT(1, sym_length->attr.entry);

    Symbol *sym_ext = get_symbol(&asm_ctx.symbols, "EXTL3");
    TEST_ASSERT_NOT_NULL(sym_ext);
    TEST_ASSERT_EQUAL_INT(1, sym_ext->attr.external);

    cleanup_test_assembler(&asm_ctx);
    cleanup_assembler_artifacts(base_name);
}

static void test_pass2_undefined_symbol_error(void)
{
    const char *base_name = "tests/fixtures/pass2_undef_sym";
    char as_path[512];
    char ob_path[512];

    snprintf(ob_path, sizeof(ob_path), "%s.ob", base_name);

    /* References UNDEFVAR which is never declared */
    const char *source =
        "MAIN: mov UNDEFVAR, r1\n"
        "      hlt\n";

    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", source);

    FILE *fp = fopen(as_path, "r");
    TEST_ASSERT_NOT_NULL(fp);

    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, base_name);

    bool p1_ok = assembler_pass1(fp, &asm_ctx);
    TEST_ASSERT_TRUE(p1_ok);

    fseek(fp, 0, SEEK_SET);
    assembler_pass2(fp, &asm_ctx);
    fclose(fp);

    /* Pass 2 failed to resolve symbol -> NO object file should be created */
    TEST_ASSERT_FALSE(file_exists(ob_path));

    cleanup_test_assembler(&asm_ctx);
    cleanup_assembler_artifacts(base_name);
}

static void test_pass2_extern_declared_as_entry_error(void)
{
    const char *base_name = "tests/fixtures/pass2_ext_ent_conflict";
    char as_path[512];
    char ob_path[512];

    snprintf(ob_path, sizeof(ob_path), "%s.ob", base_name);

    /* EXTSYM is declared as both extern and entry */
    const char *source =
        ".extern EXTSYM\n"
        ".entry EXTSYM\n"
        "MAIN: mov r1, r2\n"
        "      hlt\n";

    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", source);

    FILE *fp = fopen(as_path, "r");
    TEST_ASSERT_NOT_NULL(fp);

    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, base_name);

    bool p1_ok = assembler_pass1(fp, &asm_ctx);
    TEST_ASSERT_TRUE(p1_ok);

    fseek(fp, 0, SEEK_SET);
    assembler_pass2(fp, &asm_ctx);
    fclose(fp);

    /* Conflict should prevent output generation */
    TEST_ASSERT_FALSE(file_exists(ob_path));

    cleanup_test_assembler(&asm_ctx);
    cleanup_assembler_artifacts(base_name);
}

void run_assembler_pass2_tests(void)
{
    TEST_SUITE_START("Assembler Pass 2 Integration");
    RUN_TEST(test_pass2_symbol_resolution_and_file_creation);
    RUN_TEST(test_pass2_undefined_symbol_error);
    RUN_TEST(test_pass2_extern_declared_as_entry_error);
}
