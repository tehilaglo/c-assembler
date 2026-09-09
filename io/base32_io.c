/**
 * @file base32_io.c
 * @brief Utilities for writing values in the assembler's custom base-32 format.
 *
 * This module provides output helpers for encoding numeric values using the
 * project's custom base-32 representation. In particular, it writes a 10-bit
 * address as two base-32 symbols:
 * - the five most-significant bits,
 * - the five least-significant bits.
 *
 * @author Tehila Cahnaman
 */

#include "base32_io.h"

#include <stdio.h>

#include "isa.h"

/**
 * @brief Writes an address to a file using two custom base-32 symbols.
 *
 * The address is split into:
 * - the five most-significant bits,
 * - the five least-significant bits.
 *
 * Each 5-bit group is converted into its custom base-32 symbol and written
 * to the given output stream.
 *
 * @param fp Output stream to write to.
 * @param address Address value to encode.
 */
void f_put_b32_address(FILE *fp, const int address)
{
    if (fp == NULL)
        return;

    /* Write the five most-significant bits. */
    fputc(get_b32_symbol((char)((address >> 5) & 31)), fp);

    /* Write the five least-significant bits. */
    fputc(get_b32_symbol((char)(address & 31)), fp);
}
