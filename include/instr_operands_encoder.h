/**
 * @file instr_operands_encoder.h
 * @brief Declarations for encoding instruction operands into machine words for the assembler.
 *
 * @author Tehila Cahnaman
 */

#pragma once

#include <stdbool.h>

#include "assembler.h"
#include "isa.h"
#include "macro_table.h"

/**
 * @def IMDT_MAX_SIZE
 * @brief Maximum number of bits supported by an immediate operand value.
 *
 * Immediate values exceeding this width may be truncated during encoding.
 */
#define IMDT_MAX_SIZE 8


/**
 * @struct InstrCtx
 * @brief Context used while encoding a single assembly instruction line.
 *
 * This structure stores all intermediate information required while
 * translating operands of one instruction into machine words. It keeps
 * track of parsed operands, allowed addressing modes, emitted words,
 * and temporary buffers used during parsing.
 */
typedef struct
{
    /* Immutable inputs */

    char *buf;   /**< Remaining operand text from the source line. */
    char *instr; /**< Instruction mnemonic. */

    /* Running encoding state */

    int ic_count; /**< Number of additional instruction words emitted. */
    bool error;   /**< Indicates whether any recoverable error occurred. */

    /* Addressing modes allowed by the current opcode */

    bool src_exists;  /**< Whether the instruction expects a source operand. */
    bool dest_exists; /**< Whether the instruction expects a destination operand. */

    bool src_mask[4];  /**< Allowed addressing modes for the source operand. */
    bool dest_mask[4]; /**< Allowed addressing modes for the destination operand. */

    /* Parsed operand details */

    int src_reg;  /**< Parsed source register number (if applicable). */
    int dest_reg; /**< Parsed destination register number (if applicable). */

    char src_op;  /**< Encoded addressing type for the source operand. */
    char dest_op; /**< Encoded addressing type for the destination operand. */

    bool src_is_reg;  /**< True if the source operand is a register. */
    bool dest_is_reg; /**< True if the destination operand is a register. */

    /* Scratch buffer reused for parsed symbol operands */

    char symbol[MAX_LINE_LEN - MAX_INSTR_LEN]; /**< Temporary symbol buffer. */

} InstrCtx;


/**
 * @brief Encoded addressing modes used during operand encoding.
 */
enum
{
    IMMDT_ADDRESS = 0, /**< Immediate addressing (#number). */
    DIRECT_ADDRESS,    /**< Direct symbol addressing (symbol). */
    STRUCT_ADDRESS,    /**< Struct field addressing (symbol.field). */
    REG_ADDRESS        /**< Register addressing (r0–r7). */
};


/**
 * @brief Index values used to access operand addressing masks.
 *
 * These values correspond to entries in the src_mask[] and dest_mask[]
 * arrays inside the InstrCtx structure.
 */
enum
{
    IM = 0, /**< Immediate addressing (#number). */
    DT,     /**< Direct symbol addressing (symbol). */
    ST,     /**< Struct addressing (symbol.field). */
    RG      /**< Register addressing (r0–r7). */
};


/**
 * @brief Encodes the source operand of a two-operand instruction.
 *
 * @param asm Pointer to the assembler state.
 * @param ictx Pointer to the instruction-encoding context.
 * @return true if processing completed structurally, false on fatal failure.
 */
bool encode_source_operand(const Assembler *asm, InstrCtx *ictx);


/**
 * @brief Encodes the destination operand of an instruction.
 *
 * @param asm Pointer to the assembler state.
 * @param ictx Pointer to the instruction-encoding context.
 * @return true if processing completed structurally, false on fatal failure.
 */
bool encode_dest_operand(const Assembler *asm, InstrCtx *ictx);
