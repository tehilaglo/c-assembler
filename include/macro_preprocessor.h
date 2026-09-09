/**
* @file macro_preprocessor.h
 * @brief Macro preprocessing declarations for the assembler.
 *
 * @author Tehila Cahnaman
 */

#pragma once

#include <stdio.h>

/**
 * @brief Preprocesses an assembly source file and expands user-defined macros.
 *
 * @param fp Input assembly file stream.
 * @param file_name Base file name without extension.
 * @return FILE pointer to the generated preprocessed file on success,
 *         or NULL if preprocessing failed.
 */
FILE *preprocess(FILE *fp, char *file_name);
