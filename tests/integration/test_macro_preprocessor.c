#include "test_framework.h"
#include "test_helpers.h"
#include "macro_preprocessor.h"

static void test_preprocessor_no_macros(void)
{
    const char *base_name = "tests/fixtures/prepro_no_macro";
    char as_path[512];
    char am_path[512];

    const char *source =
        "MAIN: mov r1, r2\n"
        "      add #5, r3\n"
        "      hlt\n";

    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", source);
    snprintf(am_path, sizeof(am_path), "%s.am", base_name);

    FILE *fp = fopen(as_path, "r");
    TEST_ASSERT_NOT_NULL(fp);

    FILE *am_fp = preprocess(fp, (char *)base_name);
    fclose(fp);
    TEST_ASSERT_NOT_NULL(am_fp);
    fclose(am_fp);

    char *am_content = read_file_content(am_path);
    TEST_ASSERT_NOT_NULL(am_content);
    TEST_ASSERT_EQUAL_STR(source, am_content);

    free(am_content);
    cleanup_assembler_artifacts(base_name);
}

static void test_preprocessor_one_macro(void)
{
    const char *base_name = "tests/fixtures/prepro_one_macro";
    char as_path[512];
    char am_path[512];

    const char *source =
        "macro m1\n"
        "    mov r1, r2\n"
        "    add r3, r4\n"
        "endmacro\n"
        "MAIN: mov r0, r0\n"
        "m1\n"
        "      hlt\n";

    const char *expected =
        "MAIN: mov r0, r0\n"
        "    mov r1, r2\n"
        "    add r3, r4\n"
        "      hlt\n";

    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", source);
    snprintf(am_path, sizeof(am_path), "%s.am", base_name);

    FILE *fp = fopen(as_path, "r");
    TEST_ASSERT_NOT_NULL(fp);

    FILE *am_fp = preprocess(fp, (char *)base_name);
    fclose(fp);
    TEST_ASSERT_NOT_NULL(am_fp);
    fclose(am_fp);

    char *am_content = read_file_content(am_path);
    TEST_ASSERT_NOT_NULL(am_content);
    TEST_ASSERT_EQUAL_STR(expected, am_content);

    free(am_content);
    cleanup_assembler_artifacts(base_name);
}

static void test_preprocessor_multiple_macros_and_invocations(void)
{
    const char *base_name = "tests/fixtures/prepro_multi_macro";
    char as_path[512];
    char am_path[512];

    const char *source =
        "macro m1\n"
        "    inc r1\n"
        "endmacro\n"
        "macro m2\n"
        "    dec r2\n"
        "endmacro\n"
        "START: mov r0, r0\n"
        "m1\n"
        "m2\n"
        "m1\n"
        "END: hlt\n";

    const char *expected =
        "START: mov r0, r0\n"
        "    inc r1\n"
        "    dec r2\n"
        "    inc r1\n"
        "END: hlt\n";

    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", source);
    snprintf(am_path, sizeof(am_path), "%s.am", base_name);

    FILE *fp = fopen(as_path, "r");
    TEST_ASSERT_NOT_NULL(fp);

    FILE *am_fp = preprocess(fp, (char *)base_name);
    fclose(fp);
    TEST_ASSERT_NOT_NULL(am_fp);
    fclose(am_fp);

    char *am_content = read_file_content(am_path);
    TEST_ASSERT_NOT_NULL(am_content);
    TEST_ASSERT_EQUAL_STR(expected, am_content);

    free(am_content);
    cleanup_assembler_artifacts(base_name);
}

static void test_preprocessor_invalid_declarations(void)
{
    const char *base_name = "tests/fixtures/prepro_invalid";
    char as_path[512];

    /* Missing macro name */
    const char *src1 =
        "macro\n"
        "    mov r1, r2\n"
        "endmacro\n";

    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", src1);
    FILE *fp = fopen(as_path, "r");
    TEST_ASSERT_NOT_NULL(fp);
    FILE *am_fp = preprocess(fp, (char *)base_name);
    fclose(fp);
    TEST_ASSERT_NULL(am_fp);
    cleanup_assembler_artifacts(base_name);

    /* Duplicate macro name */
    const char *src2 =
        "macro my_mac\n"
        "    inc r1\n"
        "endmacro\n"
        "macro my_mac\n"
        "    dec r2\n"
        "endmacro\n";

    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", src2);
    fp = fopen(as_path, "r");
    TEST_ASSERT_NOT_NULL(fp);
    am_fp = preprocess(fp, (char *)base_name);
    fclose(fp);
    TEST_ASSERT_NULL(am_fp);
    cleanup_assembler_artifacts(base_name);

    /* Reserved macro name (instruction mnemonic) */
    const char *src3 =
        "macro mov\n"
        "    add r1, r2\n"
        "endmacro\n";

    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", src3);
    fp = fopen(as_path, "r");
    TEST_ASSERT_NOT_NULL(fp);
    am_fp = preprocess(fp, (char *)base_name);
    fclose(fp);
    TEST_ASSERT_NULL(am_fp);
    cleanup_assembler_artifacts(base_name);

    /* Reserved macro name (register name) */
    const char *src4 =
        "macro r1\n"
        "    add r1, r2\n"
        "endmacro\n";

    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", src4);
    fp = fopen(as_path, "r");
    TEST_ASSERT_NOT_NULL(fp);
    am_fp = preprocess(fp, (char *)base_name);
    fclose(fp);
    TEST_ASSERT_NULL(am_fp);
    cleanup_assembler_artifacts(base_name);

    /* Reserved macro name (directive keyword) */
    const char *src5 =
        "macro data\n"
        "    add r1, r2\n"
        "endmacro\n";

    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", src5);
    fp = fopen(as_path, "r");
    TEST_ASSERT_NOT_NULL(fp);
    am_fp = preprocess(fp, (char *)base_name);
    fclose(fp);
    TEST_ASSERT_NULL(am_fp);
    cleanup_assembler_artifacts(base_name);
}

static void test_preprocessor_malformed_endmacro_and_mixed_lines(void)
{
    const char *base_name = "tests/fixtures/prepro_malformed_end";
    char as_path[512];
    char am_path[512];

    /* Comments and whitespace mixed around macros */
    const char *src1 =
        "; Initial comment\n"
        "\n"
        "macro mac1\n"
        "    prn #10\n"
        "endmacro\n"
        "; Middle comment\n"
        "MAIN: mov r0, r0\n"
        "mac1\n"
        "; Trailing comment\n";

    const char *expected1 =
        "; Initial comment\n"
        "\n"
        "; Middle comment\n"
        "MAIN: mov r0, r0\n"
        "    prn #10\n"
        "; Trailing comment\n";

    create_temp_as_file(as_path, sizeof(as_path), base_name, ".as", src1);
    snprintf(am_path, sizeof(am_path), "%s.am", base_name);

    FILE *fp = fopen(as_path, "r");
    TEST_ASSERT_NOT_NULL(fp);
    FILE *am_fp = preprocess(fp, (char *)base_name);
    fclose(fp);
    TEST_ASSERT_NOT_NULL(am_fp);
    fclose(am_fp);

    char *am_content = read_file_content(am_path);
    TEST_ASSERT_NOT_NULL(am_content);
    TEST_ASSERT_EQUAL_STR(expected1, am_content);

    free(am_content);
    cleanup_assembler_artifacts(base_name);
}

void run_macro_preprocessor_tests(void)
{
    TEST_SUITE_START("Macro Preprocessor Integration");
    RUN_TEST(test_preprocessor_no_macros);
    RUN_TEST(test_preprocessor_one_macro);
    RUN_TEST(test_preprocessor_multiple_macros_and_invocations);
    RUN_TEST(test_preprocessor_invalid_declarations);
    RUN_TEST(test_preprocessor_malformed_endmacro_and_mixed_lines);
}
