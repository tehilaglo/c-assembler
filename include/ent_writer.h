/**
 * @file ent_writer.h
 * @brief Declarations for writing assembler entries files.
 *
 * @author Tehila Cahnaman
 */

#pragma once

#include "assembler.h"

/**
 * @brief Writes the entries (.ent) file for the assembled program.
 *
 * @param asm Assembler context containing the symbol table and layout data.
 */
void to_ent_file(const Assembler *asm);
