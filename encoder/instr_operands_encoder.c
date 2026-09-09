/**
 * @file instr_operands_encoder.c
 * @brief Encodes instruction operands into machine words for the assembler.
 *
 * This module is responsible for translating parsed source and destination
 * operands into encoded instruction words. It supports immediate, direct,
 * struct, and register operands, while updating the instruction counter
 * according to the number of emitted words.
 *
 * The implementation preserves bookkeeping even when operand-level parsing
 * reports recoverable errors, allowing later stages to continue processing
 * consistently.
 *
 * @author Tehila Cahnaman
 */

#include "instr_operands_encoder.h"

#include <stddef.h>
#include <string.h>

#include "assembler.h"
#include "file_config.h"
#include "logger.h"
#include "operand_parser.h"

/**
 * @brief Encodes an immediate operand into one machine word.
 *
 * The encoded word stores the immediate value in the assembler's instruction
 * word layout. A warning is emitted if the value exceeds the supported
 * immediate width and may be truncated.
 *
 * @param asm Pointer to the assembler state.
 * @param num Immediate numeric value to encode.
 * @param ic_count Pointer to the current emitted instruction-word count.
 */
static void encode_immediate_operand(const Assembler *asm,
                                     const int num,
                                     int *ic_count)
{
    if (ic_count == NULL)
    {
        return;
    }

    mem_check(asm->ic + *ic_count + asm->dc);

    /* Verify the value fits the supported immediate bit width. */
    if ((num >= 0 && ((num >> IMDT_MAX_SIZE) != 0)) ||
        (num < 0 && ((num >> IMDT_MAX_SIZE) != -1)))
    {
        log_warning("%s%s: line %d: value may be truncated — exceeds %d-bit limit\n",
                    asm->logger.file_name,
                    AUTMK_EXTENSION,
                    asm->logger.line_num,
                    IMDT_MAX_SIZE);
    }

    /*
     * The immediate value is split across the encoded word fields according to
     * the project's machine-word format.
     */
    EncodedInstruction *instruction = new_instruction(
        NULL,
        new_instr_word(ABSOLUTE,
                       (char) (num & 3),
                       (char) ((num >> 2) & 3),
                       (char) ((num >> 4) & 15)));

    asm->code[asm->ic + *ic_count] = instruction;
    (*ic_count)++;
}

/**
 * @brief Encodes a struct operand into two machine words.
 *
 * The first word stores the symbol reference, and the second stores the
 * selected struct field number.
 *
 * @param asm Pointer to the assembler state.
 * @param field Struct field index.
 * @param symbol Symbol name associated with the struct operand.
 * @param ic_count Pointer to the current emitted instruction-word count.
 */
static void encode_struct_operand(const Assembler *asm,
                                  const int field,
                                  const char *symbol,
                                  int *ic_count)
{
    if (ic_count == NULL)
    {
        return;
    }

    mem_check(asm->ic + *ic_count + asm->dc);

    /* First emitted word: unresolved/resolved symbol reference. */
    EncodedInstruction *instruction = new_instruction(symbol, NULL);
    asm->code[asm->ic + *ic_count] = instruction;
    (*ic_count)++;

    mem_check(asm->ic + *ic_count + asm->dc);

    /* Second emitted word: struct field selector. */
    instruction = new_instruction(NULL, new_instr_word(ABSOLUTE, (char)field, 0, 0));
    asm->code[asm->ic + *ic_count] = instruction;
    (*ic_count)++;
}

/**
 * @brief Encodes a direct operand into one machine word.
 *
 * The emitted word stores a symbol reference that will later be resolved.
 *
 * @param asm Pointer to the assembler state.
 * @param symbol Symbol name associated with the operand.
 * @param ic_count Pointer to the current emitted instruction-word count.
 */
static void encode_direct_operand(const Assembler *asm,
                                  char *symbol,
                                  int *ic_count)
{
    if (ic_count == NULL)
    {
        return;
    }

    mem_check(asm->ic + *ic_count + asm->dc);

    EncodedInstruction *instruction = new_instruction(symbol, NULL);
    asm->code[asm->ic + *ic_count] = instruction;
    (*ic_count)++;
}

/**
 * @brief Encodes register operands into a shared machine word.
 *
 * When either the source operand, the destination operand, or both are
 * registers, this function emits the extra machine word that stores the
 * register bits according to the instruction format.
 *
 * @param asm Pointer to the assembler state.
 * @param ictx Pointer to the instruction-encoding context.
 */
static void encode_register_operands(const Assembler *asm, InstrCtx *ictx)
{
    char dest_low = 0;
    char dest_high = 0;
    char src_bits = 0;

    if (ictx->dest_is_reg)
    {
        dest_low = (char)(ictx->dest_reg & 3);
        dest_high = (char)((ictx->dest_reg >> 2) & 3);
    }

    if (ictx->src_is_reg)
    {
        src_bits = (char)(ictx->src_reg & 15);
    }

    EncodedInstruction *instruction = new_instruction(NULL,
                                                      new_instr_word(ABSOLUTE,
                                                                     dest_low,
                                                                     dest_high,
                                                                     src_bits));

    asm->code[asm->ic + ictx->ic_count] = instruction;
    ictx->ic_count++;
}

/**
 * @brief Encodes the source operand of a two-operand instruction.
 *
 * The function tries all addressing modes permitted by the opcode, in the
 * order defined by the context masks. Fatal structural problems such as a
 * missing operand cause an immediate failure return. Operand-level syntax or
 * semantic errors set the context error flag while still allowing bookkeeping
 * to continue.
 *
 * @param asm Pointer to the assembler state.
 * @param ictx Pointer to the instruction-encoding context.
 * @return true if processing completed structurally, false on fatal failure.
 */
bool encode_source_operand(const Assembler *asm, InstrCtx *ictx)
{
    int number;
    OperandParseResult parse_result;

    if (asm == NULL || ictx == NULL || ictx->buf == NULL || ictx->instr == NULL)
    {
        return false;
    }

    /* Source operand ends at the first comma. */
    char *token = strtok(ictx->buf, ",");
    if (token == NULL)
    {
        log_error("%s%s: line %d: missing source operand for '%s'\n",
                  asm->logger.file_name,
                  AUTMK_EXTENSION,
                  asm->logger.line_num,
                  ictx->instr);
        return false;
    }

    /* Immediate operand: #number */
    if (ictx->src_mask[IM])
    {
        parse_result = parse_immediate_operand(asm, token, ictx->instr, &number);
        if (parse_result == OPERAND_OK)
        {
            ictx->src_op = IMMDT_ADDRESS;
            encode_immediate_operand(asm, number, &ictx->ic_count);
            return true;
        }

        if (parse_result == OPERAND_ERR)
        {
            ictx->error = true;
        }
    }

    /* Struct operand: symbol.field */
    if (ictx->src_mask[ST])
    {
        parse_result = parse_struct_operand(asm, token, ictx->instr, ictx->symbol, &number);
        if (parse_result == OPERAND_OK)
        {
            ictx->src_op = STRUCT_ADDRESS;
            encode_struct_operand(asm, number, ictx->symbol, &ictx->ic_count);
            return true;
        }

        if (parse_result == OPERAND_ERR)
        {
            ictx->error = true;
        }
    }

    /* Register operand: r0-r7 */
    if (ictx->src_mask[RG])
    {
        parse_result = parse_register_operand(token, &ictx->src_reg);
        if (parse_result == OPERAND_OK)
        {
            ictx->src_op = REG_ADDRESS;
            ictx->src_is_reg = true;
            return true;
        }

        if (parse_result == OPERAND_ERR)
        {
            ictx->error = true;
        }
    }

    /* Direct operand: symbol */
    if (ictx->src_mask[DT])
    {
        parse_result = parse_direct_operand(asm, token, ictx->instr, ictx->symbol);
        if (parse_result == OPERAND_OK)
        {
            ictx->src_op = DIRECT_ADDRESS;
            encode_direct_operand(asm, ictx->symbol, &ictx->ic_count);
            return true;
        }

        ictx->error = true;
    }

    return true;
}

/**
 * @brief Encodes the destination operand of an instruction.
 *
 * For two-operand instructions, parsing starts after the source operand.
 * For single-operand instructions, the full buffer is treated as the
 * destination operand. Extra commas are reported as too many operands.
 *
 * If at least one operand is register-direct, the required register machine
 * word is emitted after operand parsing.
 *
 * @param asm Pointer to the assembler state.
 * @param ictx Pointer to the instruction-encoding context.
 * @return true if processing completed structurally, false on fatal failure.
 */
bool encode_dest_operand(const Assembler *asm, InstrCtx *ictx)
{
    char *token;
    int number;
    OperandParseResult parse_result;

    if (asm == NULL || ictx == NULL || ictx->buf == NULL || ictx->instr == NULL)
    {
        return false;
    }

    /*
     * If a source operand exists, continue tokenizing after the comma.
     * Otherwise, the entire buffer is the destination operand.
     */
    token = ictx->src_exists ? strtok(NULL, "") : ictx->buf;

    if (token == NULL)
    {
        log_error("%s%s: line %d: '%s' expects %d operand(s) but fewer were found\n",
                  asm->logger.file_name,
                  AUTMK_EXTENSION,
                  asm->logger.line_num,
                  ictx->instr,
                  ictx->src_exists ? 2 : 1);
        return false;
    }

    mem_check(asm->ic + ictx->ic_count + asm->dc);

    /* Detect extra commas, which indicate too many operands. */
    const char *cursor = token;
    while (*cursor != '\0')
    {
        if (*cursor++ == ',')
        {
            log_error("%s%s: line %d: '%s' has too many operands; expected only %d\n",
                      asm->logger.file_name,
                      AUTMK_EXTENSION,
                      asm->logger.line_num,
                      ictx->instr,
                      ictx->src_exists ? 2 : 1);
            ictx->error = true;
            goto encode_registers_if_needed;
        }
    }

    /* Immediate operand: #number */
    if (ictx->dest_mask[IM])
    {
        parse_result = parse_immediate_operand(asm, token, ictx->instr, &number);
        if (parse_result == OPERAND_OK)
        {
            ictx->dest_op = IMMDT_ADDRESS;
            encode_immediate_operand(asm, number, &ictx->ic_count);
            goto encode_registers_if_needed;
        }

        if (parse_result == OPERAND_ERR)
        {
            ictx->error = true;
        }
    }

    /* Struct operand: symbol.field */
    if (ictx->dest_mask[ST])
    {
        parse_result = parse_struct_operand(asm, token, ictx->instr, ictx->symbol, &number);
        if (parse_result == OPERAND_OK)
        {
            ictx->dest_op = STRUCT_ADDRESS;
            encode_struct_operand(asm, number, ictx->symbol, &ictx->ic_count);
            goto encode_registers_if_needed;
        }

        if (parse_result == OPERAND_ERR)
        {
            ictx->error = true;
        }
    }

    /* Register operand: r0-r7 */
    if (ictx->dest_mask[RG])
    {
        parse_result = parse_register_operand(token, &ictx->dest_reg);
        if (parse_result == OPERAND_OK)
        {
            ictx->dest_op = REG_ADDRESS;
            ictx->dest_is_reg = true;
            goto encode_registers_if_needed;
        }

        if (parse_result == OPERAND_ERR)
        {
            ictx->error = true;
        }
    }

    /* Direct operand: symbol */
    if (ictx->dest_mask[DT])
    {
        parse_result = parse_direct_operand(asm, token, ictx->instr, ictx->symbol);
        if (parse_result == OPERAND_OK)
        {
            ictx->dest_op = DIRECT_ADDRESS;
            encode_direct_operand(asm, ictx->symbol, &ictx->ic_count);
        }
        else
        {
            ictx->error = true;
        }
    }

encode_registers_if_needed:
    /*
     * Register operands share a dedicated extra word. Emit it once after both
     * operands have been classified.
     */
    if (ictx->src_is_reg || ictx->dest_is_reg)
    {
        encode_register_operands(asm, ictx);
    }

    return true;
}
