#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    int tests_run;
    int tests_passed;
    int tests_failed;
    int asserts_run;
    const char *current_test_name;
    bool current_test_failed;
} TestContext;

extern TestContext g_test_ctx;

#define TEST_INIT() \
    TestContext g_test_ctx = {0, 0, 0, 0, NULL, false}

#define TEST_SUITE_START(suite_name) \
    do { \
        printf("\n========================================\n"); \
        printf("Running Suite: %s\n", suite_name); \
        printf("========================================\n"); \
    } while (0)

#define RUN_TEST(test_func) \
    do { \
        g_test_ctx.tests_run++; \
        g_test_ctx.current_test_name = #test_func; \
        g_test_ctx.current_test_failed = false; \
        test_func(); \
        if (g_test_ctx.current_test_failed) { \
            g_test_ctx.tests_failed++; \
            printf("  [FAIL] %s\n", #test_func); \
        } else { \
            g_test_ctx.tests_passed++; \
            printf("  [PASS] %s\n", #test_func); \
        } \
    } while (0)

#define TEST_ASSERT(cond) \
    do { \
        g_test_ctx.asserts_run++; \
        if (!(cond)) { \
            g_test_ctx.current_test_failed = true; \
            fprintf(stderr, "    ASSERTION FAILED: %s (%s:%d)\n", #cond, __FILE__, __LINE__); \
            return; \
        } \
    } while (0)

#define TEST_ASSERT_MSG(cond, msg) \
    do { \
        g_test_ctx.asserts_run++; \
        if (!(cond)) { \
            g_test_ctx.current_test_failed = true; \
            fprintf(stderr, "    ASSERTION FAILED: %s - %s (%s:%d)\n", #cond, msg, __FILE__, __LINE__); \
            return; \
        } \
    } while (0)

#define TEST_ASSERT_TRUE(cond) TEST_ASSERT(cond)
#define TEST_ASSERT_FALSE(cond) TEST_ASSERT(!(cond))

#define TEST_ASSERT_EQUAL_INT(expected, actual) \
    do { \
        g_test_ctx.asserts_run++; \
        long long exp_val = (long long)(expected); \
        long long act_val = (long long)(actual); \
        if (exp_val != act_val) { \
            g_test_ctx.current_test_failed = true; \
            fprintf(stderr, "    ASSERTION FAILED: expected %lld, got %lld (%s:%d)\n", \
                    exp_val, act_val, __FILE__, __LINE__); \
            return; \
        } \
    } while (0)

#define TEST_ASSERT_EQUAL_CHAR(expected, actual) \
    do { \
        g_test_ctx.asserts_run++; \
        char exp_c = (char)(expected); \
        char act_c = (char)(actual); \
        if (exp_c != act_c) { \
            g_test_ctx.current_test_failed = true; \
            fprintf(stderr, "    ASSERTION FAILED: expected '%c' (0x%02x), got '%c' (0x%02x) (%s:%d)\n", \
                    exp_c, (unsigned char)exp_c, act_c, (unsigned char)act_c, __FILE__, __LINE__); \
            return; \
        } \
    } while (0)

#define TEST_ASSERT_EQUAL_STR(expected, actual) \
    do { \
        g_test_ctx.asserts_run++; \
        const char *exp_s = (expected); \
        const char *act_s = (actual); \
        if (exp_s == NULL && act_s == NULL) { \
            /* OK */ \
        } else if (exp_s == NULL || act_s == NULL || strcmp(exp_s, act_s) != 0) { \
            g_test_ctx.current_test_failed = true; \
            fprintf(stderr, "    ASSERTION FAILED: expected \"%s\", got \"%s\" (%s:%d)\n", \
                    exp_s ? exp_s : "NULL", act_s ? act_s : "NULL", __FILE__, __LINE__); \
            return; \
        } \
    } while (0)

#define TEST_ASSERT_NULL(ptr) \
    do { \
        g_test_ctx.asserts_run++; \
        if ((ptr) != NULL) { \
            g_test_ctx.current_test_failed = true; \
            fprintf(stderr, "    ASSERTION FAILED: expected NULL, got %p (%s:%d)\n", \
                    (void *)(ptr), __FILE__, __LINE__); \
            return; \
        } \
    } while (0)

#define TEST_ASSERT_NOT_NULL(ptr) \
    do { \
        g_test_ctx.asserts_run++; \
        if ((ptr) == NULL) { \
            g_test_ctx.current_test_failed = true; \
            fprintf(stderr, "    ASSERTION FAILED: expected non-NULL pointer (%s:%d)\n", \
                    __FILE__, __LINE__); \
            return; \
        } \
    } while (0)

#define TEST_REPORT_SUMMARY() \
    do { \
        printf("\n========================================\n"); \
        printf("TEST SUMMARY\n"); \
        printf("========================================\n"); \
        printf("Total Tests:   %d\n", g_test_ctx.tests_run); \
        printf("Passed:        %d\n", g_test_ctx.tests_passed); \
        printf("Failed:        %d\n", g_test_ctx.tests_failed); \
        printf("Total Asserts: %d\n", g_test_ctx.asserts_run); \
        printf("========================================\n"); \
    } while (0)

#define TEST_EXIT_CODE() (g_test_ctx.tests_failed > 0 ? 1 : 0)

#endif /* TEST_FRAMEWORK_H */
