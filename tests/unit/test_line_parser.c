#include "test_framework.h"
#include "line_parser.h"

static void test_is_inline_ws(void)
{
    TEST_ASSERT_TRUE(is_inline_ws(' '));
    TEST_ASSERT_TRUE(is_inline_ws('\t'));
    TEST_ASSERT_FALSE(is_inline_ws('\n'));
    TEST_ASSERT_FALSE(is_inline_ws('\0'));
    TEST_ASSERT_FALSE(is_inline_ws('a'));
    TEST_ASSERT_FALSE(is_inline_ws(';'));
}

static void test_skip_inline_ws(void)
{
    char str1[] = "   \t  hello";
    char *p1 = str1;
    skip_inline_ws(&p1);
    TEST_ASSERT_EQUAL_STR("hello", p1);

    char str2[] = "hello";
    char *p2 = str2;
    skip_inline_ws(&p2);
    TEST_ASSERT_EQUAL_STR("hello", p2);

    char str3[] = "   \t  ";
    char *p3 = str3;
    skip_inline_ws(&p3);
    TEST_ASSERT_EQUAL_STR("", p3);

    skip_inline_ws(NULL);
    char *null_p = NULL;
    skip_inline_ws(&null_p);
}

static void test_skip_ws_and_comments_whitespace(void)
{
    char line_spaces[] = "    mov r1, r2";
    char *p = line_spaces;
    char *res = skip_ws_and_comments(&p);
    TEST_ASSERT_NOT_NULL(res);
    TEST_ASSERT_EQUAL_STR("mov r1, r2", res);

    char line_tabs[] = "\t\tmov r1, r2";
    p = line_tabs;
    res = skip_ws_and_comments(&p);
    TEST_ASSERT_NOT_NULL(res);
    TEST_ASSERT_EQUAL_STR("mov r1, r2", res);

    char line_mixed[] = " \t  \t mov r1, r2";
    p = line_mixed;
    res = skip_ws_and_comments(&p);
    TEST_ASSERT_NOT_NULL(res);
    TEST_ASSERT_EQUAL_STR("mov r1, r2", res);
}

static void test_skip_ws_and_comments_blank_and_comment(void)
{
    char line_blank[] = "    \t   \n";
    char *p = line_blank;
    TEST_ASSERT_NULL(skip_ws_and_comments(&p));

    char line_empty[] = "";
    p = line_empty;
    TEST_ASSERT_NULL(skip_ws_and_comments(&p));

    char line_newline[] = "\n";
    p = line_newline;
    TEST_ASSERT_NULL(skip_ws_and_comments(&p));

    char line_comment[] = "; this is a comment line";
    p = line_comment;
    TEST_ASSERT_NULL(skip_ws_and_comments(&p));

    char line_comment_ws[] = "   \t ; indented comment";
    p = line_comment_ws;
    TEST_ASSERT_NULL(skip_ws_and_comments(&p));

    TEST_ASSERT_NULL(skip_ws_and_comments(NULL));
    char *null_p = NULL;
    TEST_ASSERT_NULL(skip_ws_and_comments(&null_p));
}

static void test_extract_word_basic(void)
{
    char dest[100];
    char src[] = "MAIN: mov r1, r2";

    char *next = extract_word(dest, src);
    TEST_ASSERT_EQUAL_STR("MAIN:", dest);
    TEST_ASSERT_EQUAL_STR(" mov r1, r2", next);

    next = extract_word(dest, next + 1);
    TEST_ASSERT_EQUAL_STR("mov", dest);
    TEST_ASSERT_EQUAL_STR(" r1, r2", next);
}

static void test_extract_word_delimiters(void)
{
    char dest[100];

    /* space delimiter */
    char s1[] = "word1 word2";
    char *p = extract_word(dest, s1);
    TEST_ASSERT_EQUAL_STR("word1", dest);
    TEST_ASSERT_EQUAL_CHAR(' ', *p);

    /* tab delimiter */
    char s2[] = "word1\tword2";
    p = extract_word(dest, s2);
    TEST_ASSERT_EQUAL_STR("word1", dest);
    TEST_ASSERT_EQUAL_CHAR('\t', *p);

    /* newline delimiter */
    char s3[] = "word1\n";
    p = extract_word(dest, s3);
    TEST_ASSERT_EQUAL_STR("word1", dest);
    TEST_ASSERT_EQUAL_CHAR('\n', *p);

    /* null terminator delimiter */
    char s4[] = "single_word";
    p = extract_word(dest, s4);
    TEST_ASSERT_EQUAL_STR("single_word", dest);
    TEST_ASSERT_EQUAL_CHAR('\0', *p);

    /* empty src */
    char s5[] = "";
    p = extract_word(dest, s5);
    TEST_ASSERT_EQUAL_STR("", dest);
    TEST_ASSERT_EQUAL_CHAR('\0', *p);
}

static void test_extract_word_null_handling(void)
{
    char dest[100];
    char src[] = "test";

    TEST_ASSERT_EQUAL_STR(src, extract_word(NULL, src));
    TEST_ASSERT_NULL(extract_word(dest, NULL));
}

void run_line_parser_tests(void)
{
    TEST_SUITE_START("Line Parser");
    RUN_TEST(test_is_inline_ws);
    RUN_TEST(test_skip_inline_ws);
    RUN_TEST(test_skip_ws_and_comments_whitespace);
    RUN_TEST(test_skip_ws_and_comments_blank_and_comment);
    RUN_TEST(test_extract_word_basic);
    RUN_TEST(test_extract_word_delimiters);
    RUN_TEST(test_extract_word_null_handling);
}
