#include "test_framework.h"
#include "symbol_table.h"

static void test_symbol_creation_and_types(void)
{
    Symbol *code_sym = new_symbol("CODE_SYM", SYMBOL_CODE, 100);
    TEST_ASSERT_NOT_NULL(code_sym);
    TEST_ASSERT_EQUAL_STR("CODE_SYM", code_sym->name);
    TEST_ASSERT_EQUAL_INT(100, code_sym->address);
    TEST_ASSERT_EQUAL_INT(1, code_sym->attr.code);
    TEST_ASSERT_EQUAL_INT(0, code_sym->attr.data);
    TEST_ASSERT_EQUAL_INT(0, code_sym->attr.external);
    TEST_ASSERT_EQUAL_INT(0, code_sym->attr.entry);
    TEST_ASSERT_NULL(code_sym->next);
    free(code_sym);

    Symbol *data_sym = new_symbol("DATA_SYM", SYMBOL_DATA, 5);
    TEST_ASSERT_NOT_NULL(data_sym);
    TEST_ASSERT_EQUAL_STR("DATA_SYM", data_sym->name);
    TEST_ASSERT_EQUAL_INT(5, data_sym->address);
    TEST_ASSERT_EQUAL_INT(0, data_sym->attr.code);
    TEST_ASSERT_EQUAL_INT(1, data_sym->attr.data);
    TEST_ASSERT_EQUAL_INT(0, data_sym->attr.external);
    TEST_ASSERT_EQUAL_INT(0, data_sym->attr.entry);
    free(data_sym);

    Symbol *ext_sym = new_symbol("EXT_SYM", SYMBOL_EXT, 0);
    TEST_ASSERT_NOT_NULL(ext_sym);
    TEST_ASSERT_EQUAL_STR("EXT_SYM", ext_sym->name);
    TEST_ASSERT_EQUAL_INT(0, ext_sym->attr.code);
    TEST_ASSERT_EQUAL_INT(0, ext_sym->attr.data);
    TEST_ASSERT_EQUAL_INT(1, ext_sym->attr.external);
    TEST_ASSERT_EQUAL_INT(0, ext_sym->attr.entry);
    free(ext_sym);

    Symbol *ent_sym = new_symbol("ENT_SYM", SYMBOL_ENTRY, 20);
    TEST_ASSERT_NOT_NULL(ent_sym);
    TEST_ASSERT_EQUAL_STR("ENT_SYM", ent_sym->name);
    TEST_ASSERT_EQUAL_INT(0, ent_sym->attr.code);
    TEST_ASSERT_EQUAL_INT(0, ent_sym->attr.data);
    TEST_ASSERT_EQUAL_INT(0, ent_sym->attr.external);
    TEST_ASSERT_EQUAL_INT(1, ent_sym->attr.entry);
    free(ent_sym);
}

static void test_symbol_insertion_and_lookup(void)
{
    SymbolTable table = { NULL };

    /* Lookup on empty table */
    TEST_ASSERT_NULL(get_symbol(&table, "NONE"));
    TEST_ASSERT_FALSE(symbol_exist(&table, "NONE"));
    TEST_ASSERT_FALSE(extern_exist(&table));
    TEST_ASSERT_FALSE(entry_exist(&table));

    /* Insert into empty table */
    Symbol *sym1 = new_symbol("FIRST", SYMBOL_CODE, 100);
    insert_symbol(&table, sym1);
    TEST_ASSERT_NOT_NULL(table.head);
    TEST_ASSERT_EQUAL_STR("FIRST", table.head->name);

    /* Lookup inserted symbol */
    Symbol *found1 = get_symbol(&table, "FIRST");
    TEST_ASSERT_NOT_NULL(found1);
    TEST_ASSERT_EQUAL_STR("FIRST", found1->name);
    TEST_ASSERT_EQUAL_INT(100, found1->address);
    TEST_ASSERT_TRUE(symbol_exist(&table, "FIRST"));

    /* Insert second symbol */
    Symbol *sym2 = new_symbol("SECOND", SYMBOL_DATA, 10);
    insert_symbol(&table, sym2);
    TEST_ASSERT_EQUAL_STR("SECOND", table.head->name);
    TEST_ASSERT_EQUAL_STR("FIRST", table.head->next->name);

    Symbol *found2 = get_symbol(&table, "SECOND");
    TEST_ASSERT_NOT_NULL(found2);
    TEST_ASSERT_EQUAL_STR("SECOND", found2->name);
    TEST_ASSERT_EQUAL_INT(10, found2->address);

    /* Lookup nonexistent symbol */
    TEST_ASSERT_NULL(get_symbol(&table, "THIRD"));
    TEST_ASSERT_FALSE(symbol_exist(&table, "THIRD"));

    free_symbols(&table);
    TEST_ASSERT_NULL(table.head);
}

static void test_symbol_duplicate_name_behavior(void)
{
    SymbolTable table = { NULL };

    Symbol *first_node = new_symbol("DUP", SYMBOL_CODE, 100);
    insert_symbol(&table, first_node);

    Symbol *second_node = new_symbol("DUP", SYMBOL_DATA, 200);
    insert_symbol(&table, second_node);

    /* get_symbol returns the head node which is the most recently inserted */
    Symbol *found = get_symbol(&table, "DUP");
    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT_EQUAL_INT(200, found->address);
    TEST_ASSERT_EQUAL_INT(1, found->attr.data);

    free_symbols(&table);
}

static void test_extern_and_entry_exist(void)
{
    SymbolTable table = { NULL };

    Symbol *code_sym = new_symbol("CODE", SYMBOL_CODE, 100);
    insert_symbol(&table, code_sym);
    TEST_ASSERT_FALSE(extern_exist(&table));
    TEST_ASSERT_FALSE(entry_exist(&table));

    Symbol *ext_sym = new_symbol("EXT", SYMBOL_EXT, 0);
    insert_symbol(&table, ext_sym);
    TEST_ASSERT_TRUE(extern_exist(&table));
    TEST_ASSERT_FALSE(entry_exist(&table));

    Symbol *ent_sym = new_symbol("ENT", SYMBOL_ENTRY, 105);
    insert_symbol(&table, ent_sym);
    TEST_ASSERT_TRUE(extern_exist(&table));
    TEST_ASSERT_TRUE(entry_exist(&table));

    free_symbols(&table);
    TEST_ASSERT_FALSE(extern_exist(&table));
    TEST_ASSERT_FALSE(entry_exist(&table));
}

static void test_symbol_table_null_handling(void)
{
    TEST_ASSERT_NULL(get_symbol(NULL, "NAME"));
    SymbolTable table = { NULL };
    TEST_ASSERT_NULL(get_symbol(&table, NULL));

    TEST_ASSERT_FALSE(symbol_exist(NULL, "NAME"));
    TEST_ASSERT_FALSE(symbol_exist(&table, NULL));

    TEST_ASSERT_FALSE(extern_exist(NULL));
    TEST_ASSERT_FALSE(entry_exist(NULL));

    free_symbols(NULL);
}

void run_symbol_table_tests(void)
{
    TEST_SUITE_START("Symbol Table");
    RUN_TEST(test_symbol_creation_and_types);
    RUN_TEST(test_symbol_insertion_and_lookup);
    RUN_TEST(test_symbol_duplicate_name_behavior);
    RUN_TEST(test_extern_and_entry_exist);
    RUN_TEST(test_symbol_table_null_handling);
}
