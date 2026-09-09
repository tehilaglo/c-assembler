/**
* @file assembler_pass1.h
 * @brief First-pass declarations for the assembler pipeline.
 *
 * @author Tehila Cahnaman
 */

#pragma once

#include "assembler.h"

/**
 * @brief Executes the first pass of the assembler.
 *
 * @param fp Input file stream to read from.
 * @param asm Assembler state object to update.
 * @return true if the pass completed without errors, false otherwise.
 */
bool assembler_pass1(FILE *fp, Assembler *asm);
