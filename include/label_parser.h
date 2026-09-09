/**
 * @file label_parser.h
 * @brief Declarations for label parsing and validation helpers.
 *
 * @author Tehila Cahnaman
 */

#pragma once

#include "assembler.h"

/**
 * @enum LabelCheckResult
 * @brief Result codes returned by label parsing functions.
 */
typedef enum
{
    LABEL_VALID = 0,      /**< A valid label definition was found. */
    LABEL_NOT_PRESENT,    /**< No label definition is present in the token. */
    LABEL_INVALID         /**< A label definition exists but is invalid. */
} LabelCheckResult;

/**
 * @brief Parses and validates a label definition token.
 *
 * @param asm Assembler context used for diagnostics and symbol lookup.
 * @param buf Input token to inspect.
 * @param symbol_name Output buffer receiving the extracted label name.
 * @return LABEL_VALID, LABEL_NOT_PRESENT, or LABEL_INVALID.
 */
LabelCheckResult parse_label_definition(
    const Assembler *asm,
    const char *buf,
    char *symbol_name);
