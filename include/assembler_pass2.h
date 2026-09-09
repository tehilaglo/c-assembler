/**
 * @file assembler_pass2.h
 * @brief Second-pass declarations for the assembler pipeline.
 *
 * @author Tehila Cahnaman
 */

#pragma once

#include "assembler.h"

/**
 * @brief Executes the second pass of the assembler.
 *
 * @param fp Input file stream to read from.
 * @param asm Assembler state to update and emit from.
 */
void assembler_pass2(FILE *fp, Assembler *asm);
