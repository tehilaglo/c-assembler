/**
* @file ext_writer.h
 * @brief Declarations for writing assembler externals files.
 *
 * This module exposes the interface used to generate the externals (.ext)
 * file, which contains all references to external symbols found in the
 * assembled program.
 *
 * @author Tehila Cahnaman
 */

#pragma once

#include "assembler.h"

/**
 * @brief Writes the externals (.ext) file for the assembled program.
 *
 * @param asm Assembler context containing the symbol table and code image.
 */
void to_ext_file(const Assembler *asm);
