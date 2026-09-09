/**
 * @file opcode_rules.h
 * @brief Declarations for instruction operand classification rules.
 *
 * @author Tehila Cahnaman
 */

#pragma once

#include <stdbool.h>

#include "instr_operands_encoder.h"

/**
 * @brief Classifies an instruction and sets its legal operand rules.
 *
 * The function updates the given instruction context with the expected operand
 * presence and the legal addressing modes for source and destination operands.
 *
 * @param ictx Instruction context to classify and update.
 * @return true if the instruction mnemonic is recognized, false otherwise.
 */
bool classify_opcode(InstrCtx *ictx);
