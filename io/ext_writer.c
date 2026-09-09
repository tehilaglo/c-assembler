/**
 * @file ext_writer.c
 * @brief Writes the assembler externals output file (.ext).
 *
 * This module scans the symbol table for external symbols and searches the
 * instruction image for references to those symbols. For each occurrence,
 * it writes the symbol name and the corresponding instruction address to the
 * externals output file in the required format.
 *
 * If no external symbol usages are found, the generated file is removed.
 *
 * @author Tehila Cahnaman
 */

#include "ext_writer.h"

#include <stdio.h>
#include <string.h>

#include "assembler.h"
#include "base32_io.h"
#include "file_config.h"
#include "logger.h"

/**
 * @brief Writes a padded external symbol entry line to the output file.
 *
 * The output format is:
 * <symbol-name><spaces><tab><base32-address><newline>
 *
 * @param ext_file Open externals output file.
 * @param symbol_name Name of the external symbol.
 * @param address Instruction address where the symbol is referenced.
 */
static void write_external_entry(FILE *ext_file,
                                 const char *symbol_name,
                                 const int address)
{
    fputs(symbol_name, ext_file);

    const size_t name_len = strlen(symbol_name);

    for (int i = 1; i <= MAX_LABEL_LEN - (int)name_len; ++i)
        fputc(' ', ext_file);

    fputc('\t', ext_file);
    f_put_b32_address(ext_file, address);
    fputc('\n', ext_file);
}

/**
 * @brief Creates the externals file name and opens it for writing.
 *
 * @param asm Assembler context containing the source file name.
 * @param file_name Buffer that receives the generated file name.
 * @param file_name_size Size of @p file_name in bytes.
 * @return Open file stream for the externals file.
 */
static FILE *open_ext_file(const Assembler *asm,
                           char *file_name,
                           size_t file_name_size)
{
    snprintf(file_name, file_name_size, "%s%s", asm->logger.file_name, EXT_EXTENSION);

    FILE *ext_file = fopen(file_name, "w");
    if (ext_file == NULL)
        fatal_error("failed to open file '%s'", file_name);

    return ext_file;
}

/**
 * @brief Writes all references to a given external symbol found in the code image.
 *
 * For each code word that references the provided symbol name, an entry is
 * written to the externals file.
 *
 * @param ext_file Open externals output file.
 * @param asm Assembler context containing the encoded instruction image.
 * @param symbol External symbol whose usages should be written.
 */
static void write_symbol_occurrences(FILE *ext_file,
                                     const Assembler *asm,
                                     const Symbol *symbol)
{
    if (ext_file == NULL || symbol == NULL)
        return;

    for (int i = 0; i < asm->ic; ++i)
    {
        if (asm->code[i]->symbol_name != NULL &&
            strcmp(symbol->name, asm->code[i]->symbol_name) == 0)
        {
            write_external_entry(ext_file, symbol->name, INIT_ADDRESS + i);
        }
    }
}

/**
 * @brief Writes the assembler externals (.ext) file.
 *
 * The function scans the symbol table for symbols marked as external and
 * searches the instruction image for references to them. Each occurrence is
 * written as a separate line in the output file. If no external references
 * are found, the created file is deleted.
 *
 * @param asm Assembler context containing symbol and code tables.
 */
void to_ext_file(const Assembler *asm)
{
    char file_name[MAX_FILE_NAME + sizeof(EXT_EXTENSION)];

    if (asm == NULL)
        return;

    FILE *ext_file = open_ext_file(asm, file_name, sizeof(file_name));

    const Symbol *itr = asm->symbols.head;

    while (itr != NULL)
    {
        /* Process only symbols declared as external. */
        if (itr->attr.external)
            write_symbol_occurrences(ext_file, asm, itr);

        itr = itr->next;
    }

    /* Remove the file if no external symbol references were written. */
    if (ftell(ext_file) == 0L)
        remove(file_name);

    fclose(ext_file);
}
