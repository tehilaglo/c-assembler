/**
 * @file assembler.c
 * @brief Core assembler lifecycle implementation.
 *
 * This module manages the top-level assembler workflow: initialization,
 * execution of the two assembly passes, and cleanup of all dynamically
 * allocated compilation resources.
 *
 * @author Tehila Cahnaman
 */

#include "assembler.h"

#include <stdlib.h>

#include "assembler_pass1.h"
#include "assembler_pass2.h"

/**
 * @brief Initializes the assembler's dynamic memory buffers.
 *
 * Memory is allocated for both the data image and instruction image.
 * The buffers are sized to the assembler's maximum addressable memory.
 *
 * @param asm Assembler instance to initialize.
 */
void assembler_init(Assembler *asm)
{
    if (asm == NULL)
        fatal_error("assembler initialization failed: assembler is NULL");

    asm->data = (DataWord **)malloc(sizeof(DataWord *) * MAX_FREE_MEM);
    if (asm->data == NULL)
        fatal_error("memory allocation failed during data image initialization");

    asm->code = (EncodedInstruction **)malloc(sizeof(EncodedInstruction *) * MAX_FREE_MEM);
    if (asm->code == NULL)
    {
        free(asm->data);
        asm->data = NULL;
        fatal_error("memory allocation failed during code image initialization");
    }
}

/**
 * @brief Compiles a preprocessed assembly source file.
 *
 * The function creates a local assembler context, initializes its memory,
 * runs the first pass, and, if successful, continues to the second pass.
 * All allocated memory is released before returning.
 *
 * @param fp Input file stream to compile.
 * @param file_name Source file base name used for diagnostics/output.
 */
void assembler_compile(FILE *fp, char *file_name)
{
    Assembler assembler = {
        .code = NULL,
        .ic = 0,
        .data = NULL,
        .dc = 0,
        .symbols = { NULL },
        .logger = { .file_name = file_name, .line_num = 0 }
    };

    if (fp == NULL || file_name == NULL)
        fatal_error("assembler compilation failed: file pointer or file name is NULL");

    assembler_init(&assembler);

    if (!assembler_pass1(fp, &assembler))
        goto cleanup;

    assembler_pass2(fp, &assembler);

cleanup:
    free_mem(&assembler);
}

/**
 * @brief Releases all dynamically allocated assembler memory.
 *
 * The function frees:
 * - all symbols,
 * - each allocated data word,
 * - each allocated encoded instruction,
 * - nested buffers owned by encoded instructions.
 *
 * @param asm Assembler instance to clean up.
 */
void free_mem(Assembler *asm)
{
    int i;

    if (asm == NULL)
        return;

    free_symbols(&asm->symbols);

    if (asm->data != NULL)
    {
        for (i = 0; i < asm->dc; ++i)
            free(asm->data[i]);

        free(asm->data);
        asm->data = NULL;
    }

    if (asm->code != NULL)
    {
        for (i = 0; i < asm->ic; ++i)
        {
            if (asm->code[i] != NULL)
            {
                /* These members are heap-allocated by the encoding pipeline when needed. */
                free(asm->code[i]->machine_code);
                free(asm->code[i]->symbol_name);
                free(asm->code[i]);
            }
        }

        free(asm->code);
        asm->code = NULL;
    }
}

/**
 * @brief Verifies that memory usage stays within assembler capacity.
 *
 * This helper is intended to guard instruction/data growth against the
 * project's fixed maximum memory size.
 *
 * @param num Memory index or size to validate.
 */
void mem_check(const int num)
{
    if (num >= MAX_FREE_MEM)
        fatal_error("memory is full. terminating program...");
}
