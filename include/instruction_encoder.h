/**
 * @file instruction_encoder.h
 * @brief Declarations for encoding assembler instruction lines into machine-code words.
 *
 * @author Tehila Cahnaman
 */

#pragma once

#include <stdbool.h>

#include "assembler.h"

/**
 * @brief Encodes a parsed instruction line into machine code.
 *
 * @param asm Pointer to the assembler state.
 * @param buf Mutable line buffer containing the instruction operands.
 * @param instr Instruction mnemonic to encode.
 * @return true on successful processing, false if a fatal encoding failure occurred.
 */
bool encode_instruction(Assembler *asm, char *buf, char *instr);
