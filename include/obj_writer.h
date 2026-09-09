/**
 * @file obj_writer.h
 * @brief Declarations for writing assembler object files.
 *
 * @author Tehila Cahnaman
 */

#pragma once

#include "assembler.h"

/**
 * @brief Writes the assembled program into an object (.ob) file.
 *
 * @param asm Assembler context containing the program image to write.
 */
void to_obj_file(const Assembler *asm);
