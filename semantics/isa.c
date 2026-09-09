/**
 * @file isa.c
 * @brief Instruction-set tables and encoding helpers for the assembler.
 *
 * This module provides:
 * - global instruction, register, and base-32 lookup tables,
 * - allocation helpers for instruction and data machine words,
 * - ISA lookup utilities for opcodes, mnemonics, registers, and base-32 symbols.
 *
 * @author Tehila Cahnaman
 */

#include "isa.h"

#include <stdlib.h>
#include <string.h>

#include "logger.h"

/**
 * @brief Static instruction table for the target machine ISA.
 */
const InstructionDef instructions[INSTR_SET] = {
    { MOV_INSTR, 0 }, { CMP_INSTR, 1 }, { ADD_INSTR, 2 }, { SUB_INSTR, 3 },
    { NOT_INSTR, 4 }, { CLR_INSTR, 5 }, { LEA_INSTR, 6 }, { INC_INSTR, 7 },
    { DEC_INSTR, 8 }, { JMP_INSTR, 9 }, { BNE_INSTR, 10 }, { GET_INSTR, 11 },
    { PRN_INSTR, 12 }, { JSR_INSTR, 13 }, { RTS_INSTR, 14 }, { HLT_INSTR, 15 }
};

/**
 * @brief Static lookup table for the assembler's custom base-32 alphabet.
 */
const Base32Entry b32Symbols[B32_MAX_VALUE] = {
    { 0, '!' },  { 1, '@' },  { 2, '#' },  { 3, '$' },  { 4, '%' },  { 5, '^' },  { 6, '&' },  { 7, '*' },
    { 8, '<' },  { 9, '>' },  { 10, 'a' }, { 11, 'b' }, { 12, 'c' }, { 13, 'd' }, { 14, 'e' }, { 15, 'f' },
    { 16, 'g' }, { 17, 'h' }, { 18, 'i' }, { 19, 'j' }, { 20, 'k' }, { 21, 'l' }, { 22, 'm' }, { 23, 'n' },
    { 24, 'o' }, { 25, 'p' }, { 26, 'q' }, { 27, 'r' }, { 28, 's' }, { 29, 't' }, { 30, 'u' }, { 31, 'v' }
};

/**
 * @brief Static register-name table.
 */
const char *registers[REGISTERS_NUM] = {
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
};

/**
 * @brief Allocates and initializes an encoded instruction entry.
 *
 * The instruction may hold:
 * - only a machine word,
 * - only a symbol reference,
 * - or both.
 *
 * At least one of @p symbol_name or @p instr_word must be non-NULL.
 *
 * @param symbol_name Optional symbol name referenced by the instruction.
 * @param instr_word Optional machine-word payload for the instruction.
 * @return Newly allocated encoded instruction entry.
 */
EncodedInstruction *new_instruction(const char *symbol_name, InstructionWord *instr_word)
{
    if (symbol_name == NULL && instr_word == NULL)
        fatal_error("instruction creation failed - both symbol name and machine code are NULL");

    EncodedInstruction *instruction = malloc(sizeof(EncodedInstruction));
    if (instruction == NULL)
        fatal_error("failed to allocate memory for encoded instruction");

    instruction->symbol_name = NULL;
    instruction->machine_code = instr_word;

    if (symbol_name != NULL)
    {
        size_t name_len = strlen(symbol_name) + 1;

        instruction->symbol_name = (char *)malloc(name_len);
        if (instruction->symbol_name == NULL)
        {
            free(instruction);
            fatal_error("failed to allocate memory for instruction symbol name");
        }

        memcpy(instruction->symbol_name, symbol_name, name_len);
    }

    return instruction;
}

/**
 * @brief Allocates and initializes an encoded data word.
 *
 * The 10-bit representation is split into two 5-bit fields.
 *
 * @param num Numeric data value to encode.
 * @return Newly allocated data word.
 */
DataWord *new_data_word(short num)
{
    DataWord *data_word = malloc(sizeof(DataWord));

    if (data_word == NULL)
        fatal_error("memory allocation failed while creating data word for value %d", num);

    /* Store the lower 5 bits first, then the upper 5 bits. */
    data_word->low = num & 31;
    num >>= 5;
    data_word->high = num & 31;

    return data_word;
}

/**
 * @brief Allocates and initializes an instruction machine word.
 *
 * Bit masking preserves the field widths expected by the target machine:
 * - ARE: 2 bits
 * - destination mode: 2 bits
 * - source mode: 2 bits
 * - opcode: 4 bits
 *
 * @param are ARE field value.
 * @param dest_op Destination addressing mode.
 * @param src_op Source addressing mode.
 * @param op_code Opcode value.
 * @return Newly allocated instruction word.
 */
InstructionWord *new_instr_word(const char are,
                                const char dest_op,
                                const char src_op,
                                const char op_code)
{
    InstructionWord *instr_word = malloc(sizeof(InstructionWord));

    if (instr_word == NULL)
        fatal_error("failed to allocate memory for instruction word (opcode: %d)", op_code);

    instr_word->are = (unsigned short)(are & 3);
    instr_word->dst_addr_mode = (unsigned short)(dest_op & 3);
    instr_word->src_addr_mode = (unsigned short)(src_op & 3);
    instr_word->opcode = (unsigned short)(op_code & 15);

    return instr_word;
}

/**
 * @brief Returns the opcode associated with an instruction mnemonic.
 *
 * @param instr Instruction mnemonic to look up.
 * @return Matching opcode, or (uint8_t)-1 if the mnemonic is not found.
 */
uint8_t isa_get_opcode(const char *instr)
{
    if (instr == NULL)
        return (uint8_t)-1;

    for (int i = 0; i < INSTR_SET; ++i)
    {
        if (strcmp(instr, instructions[i].instr) == 0)
            return instructions[i].opcode;
    }

    return (uint8_t)-1;
}

/**
 * @brief Returns the custom base-32 symbol for a numeric value.
 *
 * @param num Numeric value to convert.
 * @return Base-32 symbol character, or -1 if the value is invalid.
 */
char get_b32_symbol(const char num)
{
    for (int i = 0; i < B32_MAX_VALUE; ++i)
    {
        if (b32Symbols[i].value == (uint8_t)num)
            return b32Symbols[i].symbol;
    }

    return -1;
}

/**
 * @brief Returns the instruction mnemonic at a given instruction-table index.
 *
 * @param index Instruction-table index.
 * @return Instruction mnemonic string, or NULL if the index is invalid.
 */
const char *isa_get_instr_name(const int index)
{
    if (index >= 0 && index < INSTR_SET)
        return instructions[index].instr;

    return NULL;
}

/**
 * @brief Returns the register name at a given register index.
 *
 * @param index Register index.
 * @return Register name string, or NULL if the index is invalid.
 */
const char *isa_get_register(const int index)
{
    if (index >= 0 && index < REGISTERS_NUM)
        return registers[index];

    return NULL;
}
