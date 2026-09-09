/**
 * @file data_directive_encoder.h
 * @brief Encodes .data, .string, and .struct directive payloads into data words.
 *
 * @author Tehila Cahnaman
 */

#pragma once

#include "assembler.h"

/**
 * @def WORD_SIZE
 * @brief Maximum number of bits preserved in a data word value.
 */
#define WORD_SIZE 10

/**
 * @enum DataEncodeStatus
 * @brief Result codes returned by data-directive encoding functions.
 */
typedef enum
{
    DATA_ENCODE_OK = 0, /**< Encoding completed successfully. */
    DATA_ENCODE_ERR     /**< Encoding failed due to invalid input. */
} DataEncodeStatus;

/**
 * @brief Encodes a .data, .string, or .struct directive payload.
 *
 * @param asm Assembler context that owns the target data image.
 * @param buf Mutable buffer containing the directive payload text.
 * @param data_type Directive token describing the payload type.
 * @return DATA_ENCODE_OK on success, or DATA_ENCODE_ERR on failure.
 */
DataEncodeStatus encode_dss(Assembler *asm, char *buf, char *data_type);
