#include "test_framework.h"
#include "macro_table.h"

static void test_macro_creation(void)
{
    Macro *m = new_macro("m1", 10, 50);
    TEST_ASSERT_NOT_NULL(m);
    TEST_ASSERT_EQUAL_STR("m1", m->name);
    TEST_ASSERT_EQUAL_INT(10, m->start_offset);
    TEST_ASSERT_EQUAL_INT(50, m->end_offset);
    TEST_ASSERT_NULL(m->next);
    free(m);
}

static void test_macro_table_insertion_and_exists(void)
{
    MacroTable table = { NULL };

    TEST_ASSERT_FALSE(macro_exists(&table, "m1"));
    TEST_ASSERT_NULL(get_macro(&table, "m1"));

    /* Set first macro as head */
    Macro *m1 = new_macro("m1", 10, 50);
    table.head = m1;

    TEST_ASSERT_TRUE(macro_exists(&table, "m1"));
    TEST_ASSERT_FALSE(macro_exists(&table, "m2"));

    /* Insert second macro */
    Macro *m2 = new_macro("m2", 60, 100);
    insert_macro(&table, m2);

    TEST_ASSERT_TRUE(macro_exists(&table, "m1"));
    TEST_ASSERT_TRUE(macro_exists(&table, "m2"));
    TEST_ASSERT_FALSE(macro_exists(&table, "m3"));

    free_macros(&table);
    TEST_ASSERT_NULL(table.head);
}

static void test_get_macro_invocation(void)
{
    MacroTable table = { NULL };
    Macro *m1 = new_macro("my_macro", 10, 50);
    table.head = m1;

    /* Simple invocation */
    char call1[] = "my_macro";
    Macro *found = get_macro(&table, call1);
    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT_EQUAL_STR("my_macro", found->name);

    /* Leading and trailing whitespace */
    char call2[] = "   \t my_macro \t \n";
    found = get_macro(&table, call2);
    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT_EQUAL_STR("my_macro", found->name);

    /* Nonexistent macro */
    char call3[] = "other_macro";
    found = get_macro(&table, call3);
    TEST_ASSERT_NULL(found);

    /* Extra text after invocation should be rejected */
    char call4[] = "my_macro extra_param";
    found = get_macro(&table, call4);
    TEST_ASSERT_NULL(found);

    /* Empty invocation line */
    char call5[] = "";
    TEST_ASSERT_NULL(get_macro(&table, call5));

    char call6[] = "   \t \n";
    TEST_ASSERT_NULL(get_macro(&table, call6));

    free_macros(&table);
}

static void test_macro_null_handling(void)
{
    TEST_ASSERT_FALSE(macro_exists(NULL, "m1"));
    MacroTable table = { NULL };
    TEST_ASSERT_FALSE(macro_exists(&table, NULL));

    TEST_ASSERT_NULL(get_macro(NULL, "m1"));
    TEST_ASSERT_NULL(get_macro(&table, NULL));

    free_macros(NULL);
}

void run_macro_table_tests(void)
{
    TEST_SUITE_START("Macro Table");
    RUN_TEST(test_macro_creation);
    RUN_TEST(test_macro_table_insertion_and_exists);
    RUN_TEST(test_get_macro_invocation);
    RUN_TEST(test_macro_null_handling);
}
