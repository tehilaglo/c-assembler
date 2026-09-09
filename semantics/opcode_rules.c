/**
 * @file opcode_rules.c
 * @brief Defines legal operand counts and addressing modes for instructions.
 *
 * This module classifies instruction mnemonics and configures the operand rules
 * associated with each opcode. For a recognized instruction, the classification
 * updates the instruction context with:
 * - whether source and destination operands are expected,
 * - which addressing modes are valid for each operand.
 *
 * @author Tehila Cahnaman
 */

#include "opcode_rules.h"

#include <string.h>

/**
 * @brief Marks all addressing modes as valid for a source operand.
 *
 * @param ictx Instruction context to update.
 */
static void allow_all_src_modes(InstrCtx *ictx)
{
    ictx->src_mask[IM] = true;
    ictx->src_mask[DT] = true;
    ictx->src_mask[ST] = true;
    ictx->src_mask[RG] = true;
}

/**
 * @brief Marks all addressing modes as valid for a destination operand.
 *
 * @param ictx Instruction context to update.
 */
static void allow_all_dest_modes(InstrCtx *ictx)
{
    ictx->dest_mask[IM] = true;
    ictx->dest_mask[DT] = true;
    ictx->dest_mask[ST] = true;
    ictx->dest_mask[RG] = true;
}

/**
 * @brief Marks direct, struct, and register addressing as valid destination modes.
 *
 * @param ictx Instruction context to update.
 */
static void allow_standard_dest_modes(InstrCtx *ictx)
{
    ictx->dest_mask[DT] = true;
    ictx->dest_mask[ST] = true;
    ictx->dest_mask[RG] = true;
}

/**
 * @brief Configures operand rules for a recognized instruction mnemonic.
 *
 * The function determines whether the instruction expects source and/or
 * destination operands and marks the legal addressing modes in the instruction
 * context structure.
 *
 * @param ictx Instruction context containing the instruction mnemonic and
 *             output rule masks.
 * @return true if the instruction mnemonic is recognized, false otherwise.
 */
bool classify_opcode(InstrCtx *ictx)
{
    const char *instr;

    if (!ictx || !ictx->instr)
        return false;

    instr = ictx->instr;

    /* Full two-operand arithmetic instructions. */
    if (strcmp(instr, MOV_INSTR) == 0 ||
        strcmp(instr, ADD_INSTR) == 0 ||
        strcmp(instr, SUB_INSTR) == 0)
    {
        ictx->src_exists = true;
        ictx->dest_exists = true;

        allow_all_src_modes(ictx);
        allow_standard_dest_modes(ictx);
    }
    else if (strcmp(instr, CMP_INSTR) == 0)
    {
        ictx->src_exists = true;
        ictx->dest_exists = true;

        allow_all_src_modes(ictx);
        allow_all_dest_modes(ictx);
    }
    else if (strcmp(instr, LEA_INSTR) == 0)
    {
        ictx->src_exists = true;
        ictx->dest_exists = true;

        ictx->src_mask[DT] = true;
        ictx->src_mask[ST] = true;

        allow_standard_dest_modes(ictx);
    }
    else if (strcmp(instr, NOT_INSTR) == 0 ||
             strcmp(instr, CLR_INSTR) == 0 ||
             strcmp(instr, INC_INSTR) == 0 ||
             strcmp(instr, DEC_INSTR) == 0 ||
             strcmp(instr, JMP_INSTR) == 0 ||
             strcmp(instr, BNE_INSTR) == 0 ||
             strcmp(instr, GET_INSTR) == 0 ||
             strcmp(instr, JSR_INSTR) == 0)
    {
        ictx->dest_exists = true;
        allow_standard_dest_modes(ictx);
    }
    else if (strcmp(instr, PRN_INSTR) == 0)
    {
        ictx->dest_exists = true;
        allow_all_dest_modes(ictx);
    }
    else if (strcmp(instr, RTS_INSTR) == 0 ||
             strcmp(instr, HLT_INSTR) == 0)
    {
        ictx->src_exists = false;
        ictx->dest_exists = false;
    }
    else
    {
        return false;
    }

    return true;
}
