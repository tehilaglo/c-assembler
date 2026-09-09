/**
 * @file base32_io.h
 * @brief Declarations for custom base-32 output helpers.
 *
 * @author Tehila Cahnaman
 */

#pragma once

#include <stdio.h>

/**
 * @brief Writes an address as two symbols in the custom base-32 format.
 *
 * @param fp Output stream to write to.
 * @param address Address value to encode.
 */
void f_put_b32_address(FILE *fp, int address);
