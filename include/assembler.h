/**
 * @file assembler.h
 * @brief Core assembler state and lifecycle declarations.
 *
 * @author Tehila Cahnaman
 */

#pragma once

#include <stdio.h>

#include "isa.h"
#include "logger.h"
#include "symbol_table.h"

/**
 * @def MAX_FREE_MEM
 * @brief Maximum number of memory words available to the assembler.
 */
#define MAX_FREE_MEM 156

/**
 * @struct Assembler
 * @brief Holds the mutable state of a single assembly compilation.
 *
 * The assembler context stores:
 * - encoded instruction words,
 * - encoded data words,
 * - instruction/data counters,
 * - the symbol table,
 * - logging metadata for diagnostics.
 */
typedef struct Assembler
{
    EncodedInstruction **code; /**< Instruction image. */
    int ic;                    /**< Instruction counter. */

    DataWord **data;           /**< Data image. */
    int dc;                    /**< Data counter. */

    SymbolTable symbols;       /**< Symbol table built during compilation. */
    Logger logger;             /**< Diagnostic context for errors and warnings. */
} Assembler;

/**
 * @brief Initializes the assembler's dynamic memory buffers.
 *
 * @param asm Assembler instance to initialize.
 */
void assembler_init(Assembler *asm);

/**
 * @brief Compiles a preprocessed assembly source file.
 *
 * @param fp Input file stream to compile.
 * @param file_name Source file base name used for diagnostics/output.
 */
void assembler_compile(FILE *fp, char *file_name);

/**
 * @brief Releases all dynamically allocated assembler memory.
 *
 * @param asm Assembler instance to clean up.
 */
void free_mem(Assembler *asm);

/**
 * @brief Verifies that a memory index does not exceed assembler capacity.
 *
 * @param num Memory index or size to validate.
 */
void mem_check(int num);
