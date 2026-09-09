/**
 * @file input_validator.h
 * @brief Declarations for utilities validation for assembler input files.
 *
 * Provides functions used to verify source file validity before
 * the assembler processing stages begin.
 *
 * @author Tehila Cahnaman
 */

#pragma once

#include <stdbool.h>
#include <stdio.h>

/**
 * @brief Check whether an input file contains meaningful content.
 *
 * @param fp        Open file pointer to the source file.
 * @param file_name File name (without extension).
 * @return true  if the file is empty or contains only whitespace
 * @return false if the file contains actual content
 */
bool is_empty_file(FILE *fp, const char *file_name);

/**
 * @brief Validate that the provided input filename has no extension.
 *
 * @param file_name Input filename.
 * @return true if filename is valid
 * @return false otherwise
 */
bool validate_input_filename(const char *file_name);
