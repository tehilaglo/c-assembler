/**
 * @file instruction_encoder.c
 * @brief Encodes assembler instruction lines into machine-code words.
 *
 * This module is responsible for:
 * - initializing an instruction encoding context,
 * - validating opcode classification,
 * - encoding source and destination operands,
 * - emitting the final opcode word,
 * - handling bookkeeping when encoding errors occur.
 *
 * The instruction itself is encoded only after its operands have been parsed
 * and classified. Operand encoding may emit additional machine words before
 * the base instruction word is committed.
 *
 * @author Tehila Cahnaman
 */

#include "instruction_encoder.h"

#include <stddef.h>
#include <string.h>

#include "assembler.h"
#include "file_config.h"
#include "line_parser.h"
#include "logger.h"
#include "opcode_rules.h"

/**
 * @brief Handles instruction-counter bookkeeping after an encoding error.
 *
 * When operand parsing detects an error after reserving or emitting operand
 * words, the instruction slot is explicitly cleared and the instruction
 * counter is advanced to preserve layout consistency for subsequent lines.
 *
 * @param asm Pointer to the assembler state.
 * @param error_detected Non-zero if an encoding error occurred.
 * @param emitted_word_count Number of words associated with the instruction.
 * @return true if an encoding error was handled, false otherwise.
 */
static bool handle_encode_error(Assembler *asm,
                                const int error_detected,
                                const int emitted_word_count)
{
    if (!error_detected)
        return false;

    asm->code[asm->ic] = NULL;

    /*
     * The base instruction word occupies one slot. Only advance further when
     * extra operand words were already accounted for.
     */
    if (emitted_word_count != 1)
        asm->ic += emitted_word_count;

    return true;
}

/**
 * @brief Initializes an instruction encoding context.
 *
 * The context is zero-initialized and then populated with the source buffer,
 * instruction mnemonic, and the initial word count for the opcode word.
 *
 * @param ictx Pointer to the context to initialize.
 * @param buf Mutable operand buffer for the current instruction line.
 * @param instr Instruction mnemonic being encoded.
 */
static void init_instruction_context(InstrCtx *ictx,
                                     char *buf,
                                     char *instr)
{
    memset(ictx, 0, sizeof(*ictx));
    ictx->buf = buf;
    ictx->instr = instr;
    ictx->ic_count = 1; /* Reserve one word for the opcode word itself. */
}

/**
 * @brief Encodes and stores the base instruction word.
 *
 * This function emits the opcode word after operands have already been parsed
 * and any required operand words have been reserved or emitted.
 *
 * @param asm Pointer to the assembler state.
 * @param instr Instruction mnemonic.
 * @param src_op_code Encoded source addressing mode.
 * @param dest_op_code Encoded destination addressing mode.
 * @param emitted_word_count Total number of words consumed by the instruction.
 */
static void encode_instruction_word(Assembler *asm,
                                    const char *instr,
                                    const char src_op_code,
                                    const char dest_op_code,
                                    const int emitted_word_count)
{
    if (instr == NULL)
        return;

    mem_check(asm->ic + asm->dc);

    EncodedInstruction *instruction = new_instruction(
        NULL,
        new_instr_word(
            ABSOLUTE,
            dest_op_code,
            src_op_code,
            isa_get_opcode(instr)));

    asm->code[asm->ic] = instruction;
    asm->ic += emitted_word_count;
}

/**
 * @brief Finalizes encoding of the current instruction.
 *
 * If the context indicates an encoding error, the function performs error
 * bookkeeping and aborts the commit. Otherwise, it emits the base opcode word.
 *
 * @param asm Pointer to the assembler state.
 * @param ictx Pointer to the populated instruction context.
 * @return true if the instruction was committed successfully, false otherwise.
 */
static bool commit_instruction(Assembler *asm, const InstrCtx *ictx)
{
    if (handle_encode_error(asm, ictx->error, ictx->ic_count))
        return false;

    encode_instruction_word(
        asm,
        ictx->instr,
        ictx->src_op,
        ictx->dest_op,
        ictx->ic_count);

    return true;
}

/**
 * @brief Encodes a parsed instruction line into machine code.
 *
 * The function classifies the opcode, validates operand count expectations,
 * encodes source and destination operands when applicable, and finally emits
 * the instruction word.
 *
 * @param asm Pointer to the assembler state.
 * @param buf Mutable line buffer containing the instruction operands.
 * @param instr Instruction mnemonic to encode.
 * @return true on successful processing, false if a fatal encoding failure occurred.
 */
bool encode_instruction(Assembler *asm, char *buf, char *instr)
{
    InstrCtx context;

    if (asm == NULL || buf == NULL || instr == NULL)
        return false;

    init_instruction_context(&context, buf, instr);

    if (!classify_opcode(&context))
        return false; /* Invalid mnemonic. */

    if (!context.dest_exists)
    {
        skip_inline_ws(&buf);

        if (*buf != '\n' && *buf != '\0')
        {
            log_error(
                "%s%s: line %d: '%s' has extra operands; expects none\n",
                asm->logger.file_name,
                AUTMK_EXTENSION,
                asm->logger.line_num,
                instr);
        }
    }

    if (context.src_exists && !encode_source_operand(asm, &context))
        return false;

    if (context.dest_exists && !encode_dest_operand(asm, &context))
        return false;

    if (!commit_instruction(asm, &context))
        return false;

    return true;
}
