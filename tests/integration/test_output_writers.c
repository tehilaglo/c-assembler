#include "test_framework.h"
#include "test_helpers.h"
#include "obj_writer.h"
#include "ent_writer.h"
#include "ext_writer.h"
#include "base32_io.h"

static void test_base32_address_formatting(void)
{
    const char *base_name = "tests/fixtures/b32_test";
    char path[512];
    snprintf(path, sizeof(path), "%s.txt", base_name);

    FILE *fp = fopen(path, "w");
    TEST_ASSERT_NOT_NULL(fp);

    /* Address 100: 100 >> 5 = 3 ('$'), 100 & 31 = 4 ('%') -> "$%" */
    f_put_b32_address(fp, 100);
    fputc('\n', fp);

    /* Address 0: 0 >> 5 = 0 ('!'), 0 & 31 = 0 ('!') -> "!!" */
    f_put_b32_address(fp, 0);
    fputc('\n', fp);

    /* Address 31: 0, 31 ('v') -> "!v" */
    f_put_b32_address(fp, 31);
    fputc('\n', fp);

    /* Address 32: 1 ('@'), 0 ('!') -> "@!" */
    f_put_b32_address(fp, 32);
    fputc('\n', fp);

    fclose(fp);

    char *content = read_file_content(path);
    TEST_ASSERT_NOT_NULL(content);
    TEST_ASSERT_EQUAL_STR("$%\n!!\n!v\n@!\n", content);

    free(content);
    remove_file_if_exists(path);
}

static void test_obj_writer_formatting(void)
{
    const char *base_name = "tests/fixtures/test_obj_out";
    char ob_path[512];
    snprintf(ob_path, sizeof(ob_path), "%s.ob", base_name);

    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, base_name);

    /* 1 instruction: hlt (opcode 15) -> IC = 1 (b32 symbol 1 is '@', 15 is 'f') */
    /* opcode 15 (1111), src 0 (00), dst 0 (00), are 0 (00)
     * high 5 bits: [ opcode(4) | src_high(1) ] = [ 1111 | 0 ] = 30 ('u')
     * low 5 bits:  [ src_low(1) | dst(2) | are(2) ] = [ 0 | 00 | 00 ] = 0 ('!')
     * word is "u!"
     */
    asm_ctx.code[0] = new_instruction(NULL, new_instr_word(ABSOLUTE, 0, 0, 15));
    asm_ctx.ic = 1;

    /* 1 data word: value 100 -> high = 3 ('$'), low = 4 ('%') */
    asm_ctx.data[0] = new_data_word(100);
    asm_ctx.dc = 1;

    to_obj_file(&asm_ctx);

    /* Header: \t<IC> <DC>\n -> "\t@ @\n" (IC=1 is '@', DC=1 is '@')
     * Line 1: "$%\t\tu!\n" (addr 100: "$%")
     * Line 2: "$^\t\t$%\n" (addr 101: 101>>5=3 '$', 101&31=5 '^' -> "$^")
     */
    char *ob_content = read_file_content(ob_path);
    TEST_ASSERT_NOT_NULL(ob_content);

    const char *expected =
        "\t@ @\n"
        "$%\t\tu!\n"
        "$^\t\t$%\n";

    TEST_ASSERT_EQUAL_STR(expected, ob_content);

    free(ob_content);
    cleanup_test_assembler(&asm_ctx);
    cleanup_assembler_artifacts(base_name);
}

static void test_ent_writer_formatting(void)
{
    const char *base_name = "tests/fixtures/test_ent_out";
    char ent_path[512];
    snprintf(ent_path, sizeof(ent_path), "%s.ent", base_name);

    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, base_name);
    asm_ctx.ic = 10;

    /* Code entry: LOOP at relative offset 0 -> address 100 ("$%") */
    Symbol *sym1 = new_symbol("LOOP", SYMBOL_ENTRY, 0);
    sym1->attr.code = 1;
    insert_symbol(&asm_ctx.symbols, sym1);

    /* Data entry: DATAVAR at relative offset 5 -> address 100 + 10 + 5 = 115 ("$s") */
    /* 115 >> 5 = 3 ('$'), 115 & 31 = 19 ('j') -> "$j" */
    Symbol *sym2 = new_symbol("DATAVAR", SYMBOL_ENTRY, 5);
    sym2->attr.data = 1;
    insert_symbol(&asm_ctx.symbols, sym2);

    to_ent_file(&asm_ctx);

    char *ent_content = read_file_content(ent_path);
    TEST_ASSERT_NOT_NULL(ent_content);

    /* DATAVAR (7 chars) + 23 spaces + \t + $j\n */
    /* LOOP (4 chars) + 26 spaces + \t + $%\n */
    const char *expected =
        "DATAVAR                       \t$j\n"
        "LOOP                          \t$%\n";

    TEST_ASSERT_EQUAL_STR(expected, ent_content);

    free(ent_content);
    cleanup_test_assembler(&asm_ctx);
    cleanup_assembler_artifacts(base_name);
}

static void test_ext_writer_formatting(void)
{
    const char *base_name = "tests/fixtures/test_ext_out";
    char ext_path[512];
    snprintf(ext_path, sizeof(ext_path), "%s.ext", base_name);

    Assembler asm_ctx;
    init_test_assembler(&asm_ctx, base_name);

    /* External symbol EXTL3 */
    Symbol *sym_ext = new_symbol("EXTL3", SYMBOL_EXT, 0);
    insert_symbol(&asm_ctx.symbols, sym_ext);

    /* Instruction referencing EXTL3 at index 2 (address 102: 102>>5=3 '$', 102&31=6 '&' -> "$&") */
    asm_ctx.code[0] = new_instruction(NULL, new_instr_word(ABSOLUTE, 0, 0, 0));
    asm_ctx.code[1] = new_instruction(NULL, new_instr_word(ABSOLUTE, 0, 0, 0));
    asm_ctx.code[2] = new_instruction("EXTL3", NULL);
    asm_ctx.ic = 3;

    to_ext_file(&asm_ctx);

    char *ext_content = read_file_content(ext_path);
    TEST_ASSERT_NOT_NULL(ext_content);

    /* EXTL3 (5 chars) + 25 spaces + \t + $&\n */
    const char *expected =
        "EXTL3                         \t$&\n";

    TEST_ASSERT_EQUAL_STR(expected, ext_content);

    free(ext_content);
    cleanup_test_assembler(&asm_ctx);
    cleanup_assembler_artifacts(base_name);
}

void run_output_writers_tests(void)
{
    TEST_SUITE_START("Output Writers");
    RUN_TEST(test_base32_address_formatting);
    RUN_TEST(test_obj_writer_formatting);
    RUN_TEST(test_ent_writer_formatting);
    RUN_TEST(test_ext_writer_formatting);
}
