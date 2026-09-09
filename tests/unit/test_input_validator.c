#include "test_framework.h"
#include "test_helpers.h"
#include "input_validator.h"

static void test_validate_input_filename(void)
{
    TEST_ASSERT_TRUE(validate_input_filename("program"));
    TEST_ASSERT_TRUE(validate_input_filename("examples/valid/as_example_1"));
    TEST_ASSERT_TRUE(validate_input_filename("dir.with.dots/program"));
    TEST_ASSERT_TRUE(validate_input_filename("../relative/path/file"));

    TEST_ASSERT_FALSE(validate_input_filename("program.as"));
    TEST_ASSERT_FALSE(validate_input_filename("program.txt"));
    TEST_ASSERT_FALSE(validate_input_filename("examples/valid/as_example_1.as"));
    TEST_ASSERT_FALSE(validate_input_filename(""));
    TEST_ASSERT_FALSE(validate_input_filename(NULL));
}

static void test_is_empty_file(void)
{
    char tmp_path[512];

    /* Truly empty file */
    const char *empty_base = "tests/fixtures/temp_empty";
    create_temp_as_file(tmp_path, sizeof(tmp_path), empty_base, ".as", "");
    FILE *fp = fopen(tmp_path, "r");
    TEST_ASSERT_NOT_NULL(fp);
    TEST_ASSERT_TRUE(is_empty_file(fp, empty_base));
    fclose(fp);
    remove_file_if_exists(tmp_path);

    /* Whitespace-only file */
    const char *ws_base = "tests/fixtures/temp_ws";
    create_temp_as_file(tmp_path, sizeof(tmp_path), ws_base, ".as", "   \t \n \t  \n");
    fp = fopen(tmp_path, "r");
    TEST_ASSERT_NOT_NULL(fp);
    TEST_ASSERT_TRUE(is_empty_file(fp, ws_base));
    fclose(fp);
    remove_file_if_exists(tmp_path);

    /* Non-empty valid file */
    const char *valid_base = "tests/fixtures/temp_non_empty";
    create_temp_as_file(tmp_path, sizeof(tmp_path), valid_base, ".as", "MAIN: mov r1, r2\n");
    fp = fopen(tmp_path, "r");
    TEST_ASSERT_NOT_NULL(fp);
    TEST_ASSERT_FALSE(is_empty_file(fp, valid_base));
    fclose(fp);
    remove_file_if_exists(tmp_path);

    /* NULL parameters */
    TEST_ASSERT_TRUE(is_empty_file(NULL, "file"));
    TEST_ASSERT_TRUE(is_empty_file(fp, NULL));
}

void run_input_validator_tests(void)
{
    TEST_SUITE_START("Input Validator");
    RUN_TEST(test_validate_input_filename);
    RUN_TEST(test_is_empty_file);
}
