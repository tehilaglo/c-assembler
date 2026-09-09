/**
 * @file assembler_pass2.c
 * @brief Implements the second pass of the assembler.
 *
 * This module performs symbol-resolution work that depends on information
 * collected during pass 1. It resolves encoded instruction operands that
 * reference symbols, processes `.entry` declarations, and emits the final
 * object, entry, and external files when no blocking errors are found.
 *
 * @author Tehila Cahnaman
 */

#include "assembler_pass2.h"

#include <string.h>

#include "assembler.h"
#include "ent_writer.h"
#include "ext_writer.h"
#include "file_config.h"
#include "line_parser.h"
#include "logger.h"
#include "macro_table.h"
#include "obj_writer.h"
#include "token_classifier.h"

/**
 * @brief Resolves symbol references stored in the code image.
 *
 * For each encoded instruction that still refers to a symbolic operand, this
 * function looks up the symbol in the symbol table and replaces the unresolved
 * placeholder with the final machine word.
 *
 * Address resolution rules:
 * - external symbols are encoded as external references,
 * - data symbols are addressed after the code image,
 * - code symbols are addressed relative to the initial code address.
 *
 * @param asm Assembler state containing the code image and symbol table.
 * @return true if all referenced symbols were resolved successfully,
 *         false if at least one symbol was undefined.
 */
static bool resolve_symbol_addresses(const Assembler *asm)
{
    bool has_error = false;

    for (int i = 0; i < asm->ic; ++i)
    {
        unsigned char address = 0;

        if (asm->code[i] == NULL || asm->code[i]->symbol_name == NULL)
            continue;

        Symbol *symbol = get_symbol(&asm->symbols, asm->code[i]->symbol_name);
        if (symbol == NULL)
        {
            log_error("%s%s: cannot resolve address %d: symbol '%s' is not defined\n",
                      asm->logger.file_name,
                      ASM_EXTENSION,
                      INIT_ADDRESS + i,
                      asm->code[i]->symbol_name);
            has_error = true;
            continue;
        }

        if (symbol->attr.external)
        {
            asm->code[i]->machine_code = new_instr_word(EXTERNAL, 0, 0, 0);
            continue;
        }

        if (symbol->attr.data)
            address = (unsigned char)(INIT_ADDRESS + asm->ic + symbol->address);
        else if (symbol->attr.code)
            address = (unsigned char)(INIT_ADDRESS + symbol->address);

        /*
         * Encode the resolved address into the machine word fields.
         * The masking logic preserves the original bit packing scheme.
         */
        asm->code[i]->machine_code = new_instr_word(
            RELOCATABLE,
            (char)(address & 3),
            (char)((address >> 2) & 3),
            (char)((address >> 4) & 15)
        );
    }

    return !has_error;
}

/**
 * @brief Processes `.entry` declarations from the source file.
 *
 * The function re-scans the file, detects `.entry` directives both with and
 * without a leading label, validates the referenced symbol, and marks the
 * symbol as an entry when appropriate.
 *
 * @param fp Input source file stream.
 * @param asm Assembler state containing the symbol table and logger.
 * @return true if at least one error was detected, false otherwise.
 */
static bool process_file_entries(FILE *fp, Assembler *asm)
{
    char buf[MAX_LINE_LEN + 1];
    char word[MAX_LINE_LEN + 1];
    char symbol_name[MAX_LINE_LEN + 1];
    bool has_error = false;

    fseek(fp, 0, SEEK_SET);
    asm->logger.line_num = 0;

    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        bool has_entry = false;
        char *cursor = buf;

        ++asm->logger.line_num;

        skip_inline_ws(&cursor);

        if (*cursor == '\n' || *cursor == ';' || *cursor == '\0')
            continue;

        cursor = extract_word(word, cursor);

        if (is_entry(word))
        {
            has_entry = true;
        }
        else if (word[strlen(word) - 1] == ':')
        {
            skip_inline_ws(&cursor);
            cursor = extract_word(word, cursor);

            if (is_entry(word))
                has_entry = true;
        }

        if (!has_entry)
            continue;

        skip_inline_ws(&cursor);
        extract_word(symbol_name, cursor);

        if (!symbol_exist(&asm->symbols, symbol_name))
        {
            has_error = true;
            log_error("%s%s: line %d: symbol '%s' in %s directive is undefined\n",
                      asm->logger.file_name,
                      AUTMK_EXTENSION,
                      asm->logger.line_num,
                      symbol_name,
                      ENTRY_DIRECTIVE);
            continue;
        }

        Symbol *symbol = get_symbol(&asm->symbols, symbol_name);
        if (symbol == NULL)
        {
            has_error = true;
            continue;
        }

        if (symbol->attr.external)
        {
            has_error = true;
            log_error("%s%s: line %d: %s declaration error: symbol '%s' is marked as external\n",
                      asm->logger.file_name,
                      AUTMK_EXTENSION,
                      asm->logger.line_num,
                      ENTRY_DIRECTIVE,
                      symbol_name);
            continue;
        }

        if (symbol->attr.entry)
        {
            log_warning("%s%s: line %d: duplicate %s declaration for symbol '%s'\n",
                        asm->logger.file_name,
                        AUTMK_EXTENSION,
                        asm->logger.line_num,
                        ENTRY_DIRECTIVE,
                        symbol_name);
        }
        else
        {
            symbol->attr.entry = 1;
        }
    }
    return has_error;
}

/**
 * @brief Executes the second pass of the assembler.
 *
 * The second pass performs three main steps:
 * 1. Process `.entry` directives from the source file.
 * 2. Resolve symbol references in the code image.
 * 3. Emit output files if compilation state is valid.
 *
 * Output generation rules:
 * - the object file is always produced on success,
 * - the entry file is produced only if entry symbols exist,
 * - the extern file is produced only if external symbols exist.
 *
 * @param fp Input file stream to re-scan.
 * @param asm Assembler state used for resolution and output.
 */
void assembler_pass2(FILE *fp, Assembler *asm)
{
    if (fp == NULL || asm == NULL)
        return;

    bool error_detected = process_file_entries(fp, asm);

    /* Do not emit files if pass 2 found errors or there is no output at all. */
    if (error_detected || (!asm->dc && !asm->ic))
        return;

    if (!resolve_symbol_addresses(asm))
        return;

    to_obj_file(asm);

    if (entry_exist(&asm->symbols))
        to_ent_file(asm);

    if (extern_exist(&asm->symbols))
        to_ext_file(asm);
}
