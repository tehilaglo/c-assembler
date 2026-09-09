#include "test_framework.h"
#include "test_helpers.h"
#include "data_directive_encoder.h"

static void test_data_directive_valid(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");

    char buf[] = "6, -9, 15, 0, +22";
    DataEncodeStatus status = encode_dss(&asm_ctx, buf, ".data");

    TEST_ASSERT_EQUAL_INT(DATA_ENCODE_OK, status);
    TEST_ASSERT_EQUAL_INT(5, asm_ctx.dc);

    /* 6: low=6, high=0 */
    TEST_ASSERT_EQUAL_INT(6, asm_ctx.data[0]->low);
    TEST_ASSERT_EQUAL_INT(0, asm_ctx.data[0]->high);

    /* -9: 10-bit two's complement: 1015 -> low = 23, high = 31 */
    TEST_ASSERT_EQUAL_INT(23, asm_ctx.data[1]->low);
    TEST_ASSERT_EQUAL_INT(31, asm_ctx.data[1]->high);

    /* 15: low=15, high=0 */
    TEST_ASSERT_EQUAL_INT(15, asm_ctx.data[2]->low);
    TEST_ASSERT_EQUAL_INT(0, asm_ctx.data[2]->high);

    /* 0: low=0, high=0 */
    TEST_ASSERT_EQUAL_INT(0, asm_ctx.data[3]->low);
    TEST_ASSERT_EQUAL_INT(0, asm_ctx.data[3]->high);

    /* +22: low=22, high=0 */
    TEST_ASSERT_EQUAL_INT(22, asm_ctx.data[4]->low);
    TEST_ASSERT_EQUAL_INT(0, asm_ctx.data[4]->high);

    cleanup_test_assembler(&asm_ctx);
}

static void test_data_directive_invalid(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");

    /* Redundant comma */
    char buf1[] = "1,, 2";
    TEST_ASSERT_EQUAL_INT(DATA_ENCODE_ERR, encode_dss(&asm_ctx, buf1, ".data"));

    /* Malformed number */
    char buf2[] = "1, abc, 3";
    TEST_ASSERT_EQUAL_INT(DATA_ENCODE_ERR, encode_dss(&asm_ctx, buf2, ".data"));

    /* Trailing non-numeric */
    char buf3[] = "1, 2a, 3";
    TEST_ASSERT_EQUAL_INT(DATA_ENCODE_ERR, encode_dss(&asm_ctx, buf3, ".data"));

    /* Empty values */
    char buf4[] = "";
    TEST_ASSERT_EQUAL_INT(DATA_ENCODE_OK, encode_dss(&asm_ctx, buf4, ".data"));

    cleanup_test_assembler(&asm_ctx);
}

static void test_string_directive_valid(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");

    /* Normal string */
    char buf1[] = "\"abc\"";
    TEST_ASSERT_EQUAL_INT(DATA_ENCODE_OK, encode_dss(&asm_ctx, buf1, ".string"));
    TEST_ASSERT_EQUAL_INT(4, asm_ctx.dc); /* 'a', 'b', 'c', '\0' */

    TEST_ASSERT_EQUAL_INT('a', asm_ctx.data[0]->low | (asm_ctx.data[0]->high << 5));
    TEST_ASSERT_EQUAL_INT('b', asm_ctx.data[1]->low | (asm_ctx.data[1]->high << 5));
    TEST_ASSERT_EQUAL_INT('c', asm_ctx.data[2]->low | (asm_ctx.data[2]->high << 5));
    TEST_ASSERT_EQUAL_INT('\0', asm_ctx.data[3]->low | (asm_ctx.data[3]->high << 5));

    /* Empty string */
    char buf2[] = "\"\"";
    TEST_ASSERT_EQUAL_INT(DATA_ENCODE_OK, encode_dss(&asm_ctx, buf2, ".string"));
    TEST_ASSERT_EQUAL_INT(5, asm_ctx.dc); /* + '\0' */
    TEST_ASSERT_EQUAL_INT('\0', asm_ctx.data[4]->low | (asm_ctx.data[4]->high << 5));

    cleanup_test_assembler(&asm_ctx);
}

static void test_string_directive_invalid(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");

    /* Missing opening quote */
    char buf1[] = "abc\"";
    TEST_ASSERT_EQUAL_INT(DATA_ENCODE_ERR, encode_dss(&asm_ctx, buf1, ".string"));

    /* Missing closing quote */
    char buf2[] = "\"abc";
    TEST_ASSERT_EQUAL_INT(DATA_ENCODE_ERR, encode_dss(&asm_ctx, buf2, ".string"));

    /* Trailing invalid characters */
    char buf3[] = "\"abc\" extra";
    TEST_ASSERT_EQUAL_INT(DATA_ENCODE_ERR, encode_dss(&asm_ctx, buf3, ".string"));

    cleanup_test_assembler(&asm_ctx);
}

static void test_struct_directive_valid(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");

    char buf[] = "8, \"ab\"";
    TEST_ASSERT_EQUAL_INT(DATA_ENCODE_OK, encode_dss(&asm_ctx, buf, ".struct"));
    TEST_ASSERT_EQUAL_INT(4, asm_ctx.dc); /* 8, 'a', 'b', '\0' */

    /* 8 */
    TEST_ASSERT_EQUAL_INT(8, asm_ctx.data[0]->low);
    TEST_ASSERT_EQUAL_INT(0, asm_ctx.data[0]->high);

    /* 'a' */
    TEST_ASSERT_EQUAL_INT('a', asm_ctx.data[1]->low | (asm_ctx.data[1]->high << 5));
    /* 'b' */
    TEST_ASSERT_EQUAL_INT('b', asm_ctx.data[2]->low | (asm_ctx.data[2]->high << 5));
    /* '\0' */
    TEST_ASSERT_EQUAL_INT('\0', asm_ctx.data[3]->low | (asm_ctx.data[3]->high << 5));

    cleanup_test_assembler(&asm_ctx);
}

static void test_struct_directive_invalid(void)
{
    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, "test_file");

    /* Missing numeric field */
    char buf1[] = "\"ab\"";
    TEST_ASSERT_EQUAL_INT(DATA_ENCODE_ERR, encode_dss(&asm_ctx, buf1, ".struct"));

    /* Missing string field */
    char buf2[] = "8";
    TEST_ASSERT_EQUAL_INT(DATA_ENCODE_ERR, encode_dss(&asm_ctx, buf2, ".struct"));

    /* Malformed number */
    char buf3[] = "xyz, \"ab\"";
    TEST_ASSERT_EQUAL_INT(DATA_ENCODE_ERR, encode_dss(&asm_ctx, buf3, ".struct"));

    /* Malformed string */
    char buf4[] = "8, ab";
    TEST_ASSERT_EQUAL_INT(DATA_ENCODE_ERR, encode_dss(&asm_ctx, buf4, ".struct"));

    cleanup_test_assembler(&asm_ctx);
}

void run_data_encoder_tests(void)
{
    TEST_SUITE_START("Data Directive Encoder");
    RUN_TEST(test_data_directive_valid);
    RUN_TEST(test_data_directive_invalid);
    RUN_TEST(test_string_directive_valid);
    RUN_TEST(test_string_directive_invalid);
    RUN_TEST(test_struct_directive_valid);
    RUN_TEST(test_struct_directive_invalid);
}
