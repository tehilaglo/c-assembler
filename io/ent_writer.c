/**
 * @file ent_writer.c
 * @brief Writes the assembler entries output file (.ent).
 *
 * This module scans the symbol table for symbols marked as entries and writes
 * them to the entries file together with their final resolved addresses in the
 * project's custom base-32 format.
 *
 * Entry symbol addresses are resolved as follows:
 * - code symbols are written relative to the initial memory address,
 * - data symbols are written after the instruction image.
 *
 * @author Tehila Cahnaman
 */

#include "ent_writer.h"

#include <stdio.h>
#include <string.h>

#include "assembler.h"
#include "base32_io.h"
#include "file_config.h"
#include "logger.h"

/**
 * @brief Computes the final output address of an entry symbol.
 *
 * Code symbols are located starting at the initial address. Data symbols are
 * placed after the instruction image, so their final address includes the
 * instruction counter offset.
 *
 * @param asm Assembler context containing the final instruction count.
 * @param symbol Entry symbol whose address should be resolved.
 * @return Final resolved address of the symbol.
 */
static int resolve_entry_address(const Assembler *asm, const Symbol *symbol)
{
    if (symbol->attr.code)
        return INIT_ADDRESS + symbol->address;

    if (symbol->attr.data)
        return INIT_ADDRESS + asm->ic + symbol->address;

    fatal_error(
        "entry file generation failed: unrecognized symbol type for '%s'",
        symbol->name);

    return 0;
}

/**
 * @brief Writes a single entry symbol record to the entries file.
 *
 * The output format is:
 * <symbol-name><spaces><tab><base32-address><newline>
 *
 * @param ent_file Open entries output file.
 * @param symbol_name Name of the entry symbol.
 * @param address Final resolved symbol address.
 */
static void write_entry_record(FILE *ent_file,
                               const char *symbol_name,
                               const int address)
{
    if (ent_file == NULL || symbol_name == NULL)
        return;

    fputs(symbol_name, ent_file);

    const size_t name_len = strlen(symbol_name);

    for (int i = 1; i <= MAX_LABEL_LEN - (int)name_len; ++i)
        fputc(' ', ent_file);

    fputc('\t', ent_file);
    f_put_b32_address(ent_file, address);
    fputc('\n', ent_file);
}

/**
 * @brief Writes the entries (.ent) file for the assembled program.
 *
 * The function scans the symbol table and emits one line for each symbol
 * marked as an entry, including its resolved final address.
 *
 * @param asm Assembler context containing the symbol table and instruction
 *            count.
 */
void to_ent_file(const Assembler *asm)
{
    char file_name[MAX_FILE_NAME + sizeof(ENT_EXTENSION)];

    if (asm == NULL)
        return;

    snprintf(file_name, sizeof(file_name), "%s%s", asm->logger.file_name, ENT_EXTENSION);

    FILE *ent_file = fopen(file_name, "w");

    if (ent_file == NULL)
        fatal_error("failed to open file '%s'", file_name);

    const Symbol *itr = asm->symbols.head;

    while (itr != NULL)
    {
        if (itr->attr.entry)
        {
            const int address = resolve_entry_address(asm, itr);
            write_entry_record(ent_file, itr->name, address);
        }

        itr = itr->next;
    }

    fclose(ent_file);
}
