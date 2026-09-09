/**
 * @file operand_parser.h
 * @brief Declarations for assembler operand parsing helpers.
 *
 * @author Tehila Cahnaman
 */

#pragma once

#include "assembler.h"

/**
 * @enum OperandParseResult
 * @brief Result codes returned by operand parsing functions.
 */
typedef enum
{
    OPERAND_OK = 0,      /**< Operand matched and parsed successfully. */
    OPERAND_NOT_MATCH,   /**< Parser does not apply to the given operand. */
    OPERAND_ERR          /**< Operand matched conceptually but is invalid. */
} OperandParseResult;

/**
 * @brief Parse an immediate operand.
 *
 * @param asm Assembler context used for diagnostics.
 * @param cursor Pointer to the operand text.
 * @param instr Instruction mnemonic for error reporting.
 * @param num Output parameter for the parsed numeric value.
 * @return Parsing result code.
 */
OperandParseResult parse_immediate_operand(
    const Assembler *asm,
    char *cursor,
    char *instr,
    int *num);

/**
 * @brief Parse a struct operand.
 *
 * @param asm Assembler context used for diagnostics.
 * @param buf Mutable operand buffer.
 * @param instr Instruction mnemonic for error reporting.
 * @param symbol Output buffer for the parsed symbol name.
 * @param field Output parameter for the parsed struct field.
 * @return Parsing result code.
 */
OperandParseResult parse_struct_operand(
    const Assembler *asm,
    char *buf,
    char *instr,
    char *symbol,
    int *field);

/**
 * @brief Parse a register operand.
 *
 * @param cursor Pointer to the operand text.
 * @param reg_num Output parameter for the parsed register number.
 * @return Parsing result code.
 */
OperandParseResult parse_register_operand(char *cursor, int *reg_num);

/**
 * @brief Parse a direct operand.
 *
 * @param asm Assembler context used for diagnostics.
 * @param buf Mutable operand buffer.
 * @param instr Instruction mnemonic for error reporting.
 * @param symbol Output buffer for the parsed symbol name.
 * @return Parsing result code.
 */
OperandParseResult parse_direct_operand(
    const Assembler *asm,
    char *buf,
    char *instr,
    char *symbol);
