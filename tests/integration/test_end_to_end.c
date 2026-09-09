#include "test_framework.h"
#include "test_helpers.h"
#include "assembler.h"
#include "macro_preprocessor.h"
#include "file_config.h"

static bool run_full_pipeline(const char *base_name)
{
    char as_path[512];
    snprintf(as_path, sizeof(as_path), "%s%s", base_name, ASM_EXTENSION);

    FILE *fp = fopen(as_path, "r");
    if (fp == NULL)
        return false;

    FILE *am_fp = preprocess(fp, (char *)base_name);
    fclose(fp);

    if (am_fp == NULL)
        return false;

    fseek(am_fp, 0, SEEK_SET);
    assembler_compile(am_fp, (char *)base_name);
    fclose(am_fp);

    return true;
}

static void test_e2e_small_valid_program(void)
{
    const char *base_name = "tests/fixtures/e2e_small_valid";
    char as_path[512];
    char ob_path[512];

    snprintf(ob_path, sizeof(ob_path), "%s.ob", base_name);

    const char *source =
        "MAIN: mov r1, r2\n"
        "      add #5, r3\n"
        "      hlt\n";

    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", source);

    bool ok = run_full_pipeline(base_name);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_TRUE(file_exists(ob_path));

    cleanup_assembler_artifacts(base_name);
}

static void test_e2e_all_directives_program(void)
{
    const char *base_name = "tests/fixtures/e2e_all_directives";
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
        ".extern EXTVAR\n"
        "MAIN:   mov S1.1, EXTVAR\n"
        "        add r2, STR\n"
        "LOOP:   jmp EXTVAR\n"
        "        prn #-5\n"
        "        sub r1, r4\n"
        "        inc K\n"
        "        mov S1.2, r3\n"
        "        bne LOOP\n"
        "END:    hlt\n"
        "STR:    .string \"abcdef\"\n"
        "LENGTH: .data 6, -9, 15\n"
        "K:      .data 22\n"
        "S1:     .struct 8, \"ab\"\n";

    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", source);

    bool ok = run_full_pipeline(base_name);
    TEST_ASSERT_TRUE(ok);

    TEST_ASSERT_TRUE(file_exists(ob_path));
    TEST_ASSERT_TRUE(file_exists(ent_path));
    TEST_ASSERT_TRUE(file_exists(ext_path));

    cleanup_assembler_artifacts(base_name);
}

static void test_e2e_macros_program(void)
{
    const char *base_name = "tests/fixtures/e2e_macros";
    char as_path[512];
    char ob_path[512];

    snprintf(ob_path, sizeof(ob_path), "%s.ob", base_name);

    const char *source =
        "macro mycopy\n"
        "    mov r1, r2\n"
        "    add r3, r4\n"
        "endmacro\n"
        "MAIN: mov r0, r0\n"
        "mycopy\n"
        "mycopy\n"
        "      hlt\n";

    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", source);

    bool ok = run_full_pipeline(base_name);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_TRUE(file_exists(ob_path));

    cleanup_assembler_artifacts(base_name);
}

static void test_e2e_invalid_preprocessing(void)
{
    const char *base_name = "tests/fixtures/e2e_inv_prepro";
    char as_path[512];
    char ob_path[512];

    snprintf(ob_path, sizeof(ob_path), "%s.ob", base_name);

    /* Incomplete macro declaration (missing name) */
    const char *source =
        "macro\n"
        "    mov r1, r2\n"
        "endmacro\n"
        "MAIN: mov r1, r2\n"
        "      hlt\n";

    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", source);

    bool ok = run_full_pipeline(base_name);
    TEST_ASSERT_FALSE(ok);
    TEST_ASSERT_FALSE(file_exists(ob_path));

    cleanup_assembler_artifacts(base_name);
}

static void test_e2e_invalid_pass1(void)
{
    const char *base_name = "tests/fixtures/e2e_inv_pass1";
    char as_path[512];
    char ob_path[512];

    snprintf(ob_path, sizeof(ob_path), "%s.ob", base_name);

    /* Invalid syntax in pass 1: unknown command */
    const char *source =
        "MAIN: unknowncommand r1, r2\n"
        "      hlt\n";

    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", source);

    bool ok = run_full_pipeline(base_name);
    TEST_ASSERT_TRUE(ok); /* Preprocess succeeded, but assembler_compile handled pass 1 error cleanly */
    TEST_ASSERT_FALSE(file_exists(ob_path)); /* No .ob produced */

    cleanup_assembler_artifacts(base_name);
}

static void test_e2e_invalid_pass2(void)
{
    const char *base_name = "tests/fixtures/e2e_inv_pass2";
    char as_path[512];
    char ob_path[512];

    snprintf(ob_path, sizeof(ob_path), "%s.ob", base_name);

    /* Undefined symbol in pass 2 */
    const char *source =
        ".entry NONEXISTENTLABEL\n"
        "MAIN: mov r1, r2\n"
        "      hlt\n";

    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", source);

    bool ok = run_full_pipeline(base_name);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_FALSE(file_exists(ob_path)); /* No .ob produced */

    cleanup_assembler_artifacts(base_name);
}

void run_end_to_end_tests(void)
{
    TEST_SUITE_START("End-to-End Pipeline");
    RUN_TEST(test_e2e_small_valid_program);
    RUN_TEST(test_e2e_all_directives_program);
    RUN_TEST(test_e2e_macros_program);
    RUN_TEST(test_e2e_invalid_preprocessing);
    RUN_TEST(test_e2e_invalid_pass1);
    RUN_TEST(test_e2e_invalid_pass2);
}
