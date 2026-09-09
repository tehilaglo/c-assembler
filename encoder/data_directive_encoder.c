/**
 * @file data_directive_encoder.c
 * @brief Encodes .data, .string, and .struct directive payloads into data words.
 *
 * This module translates assembler data directives into machine data words and
 * appends them to the assembler data image. Supported directives are:
 * - .data   : a comma-separated list of signed integers
 * - .string : a quoted string terminated with an implicit '\0'
 * - .struct : an integer field followed by a quoted string field
 *
 * The implementation performs syntax validation, emits diagnostics for invalid
 * definitions, and warns when numeric values may be truncated to fit the target
 * machine-word width.
 *
 * @author Tehila Cahnaman
 */

#include "data_directive_encoder.h"

#include <string.h>

#include "assembler.h"
#include "file_config.h"
#include "line_parser.h"
#include "logger.h"
#include "token_classifier.h"

/**
 * @brief Encodes a quoted string directive payload into data words.
 *
 * Leading inline whitespace is skipped before parsing. The string must begin
 * with a double quote and must terminate with a matching closing quote before
 * the end of line. Each character is encoded into a separate data word, and a
 * terminating '\0' data word is appended automatically.
 *
 * Any non-whitespace characters appearing after the closing quote are reported
 * as a syntax error.
 *
 * @param asm Assembler context that owns the data image and logger state.
 * @param cursor Pointer to the directive payload to parse.
 * @param data_type Directive name used for diagnostics.
 * @return DATA_ENCODE_OK on success, or DATA_ENCODE_ERR on failure.
 */
static DataEncodeStatus encode_string_data(Assembler *asm,
                                            char *cursor,
                                            const char *data_type)
{
    DataWord *data_word;

    if (!cursor)
        return DATA_ENCODE_ERR;

    skip_inline_ws(&cursor);

    if (*cursor != '"')
    {
        log_error(
            "%s%s: line %d: '%s' has invalid %s definition\n",
            asm->logger.file_name,
            AUTMK_EXTENSION,
            asm->logger.line_num,
            data_type,
            data_type + 1);
        return DATA_ENCODE_ERR;
    }

    ++cursor;

    while (1)
    {
        mem_check(asm->ic + asm->dc);

        if (*cursor == '\n' || *cursor == '\0')
        {
            log_error(
                "%s%s: line %d: syntax error — missing or unclosed string literal for '%s'\n",
                asm->logger.file_name,
                AUTMK_EXTENSION,
                asm->logger.line_num,
                data_type);
            return DATA_ENCODE_ERR;
        }

        if (*cursor == '"')
        {
            data_word = new_data_word('\0');
            asm->data[asm->dc++] = data_word;
            ++cursor;
            break;
        }

        data_word = new_data_word(*cursor);
        asm->data[asm->dc++] = data_word;
        ++cursor;
    }

    while (*cursor != '\0')
    {
        if (*cursor != ' ' && *cursor != '\t' && *cursor != '\n')
        {
            log_error(
                "%s%s: line %d: trailing characters found after closing quote — expected only whitespace\n",
                asm->logger.file_name,
                AUTMK_EXTENSION,
                asm->logger.line_num);
            return DATA_ENCODE_ERR;
        }

        ++cursor;
    }

    return DATA_ENCODE_OK;
}

/**
 * @brief Encodes one signed numeric value into a data word.
 *
 * Leading inline whitespace is skipped before parsing. The function accepts an
 * optional '+' or '-' sign followed by decimal digits. If any non-numeric
 * character appears inside the numeric token, the value is rejected.
 *
 * A warning is emitted if the parsed number may not fit within the configured
 * machine-word width.
 *
 * @param asm Assembler context that owns the data image and logger state.
 * @param cursor Pointer to the numeric token to parse.
 * @param data_type Directive name used for diagnostics.
 * @return DATA_ENCODE_OK on success, or DATA_ENCODE_ERR on failure.
 */
static DataEncodeStatus encode_numeric_data(Assembler *asm,
                                            char *cursor,
                                            const char *data_type)
{
    int sign = 1;
    int result = 0;
    bool digit_found = false;

    if (!cursor)
        return DATA_ENCODE_ERR;

    mem_check(asm->ic + asm->dc);
    skip_inline_ws(&cursor);

    if (*cursor == '-' || *cursor == '+')
    {
        if (*cursor == '-')
            sign = -1;
        ++cursor;
    }

    while (*cursor != ' ' && *cursor != '\t' && *cursor != '\0' && *cursor != '\n')
    {
        if (*cursor < '0' || *cursor > '9')
        {
            log_error(
                "%s%s: line %d: invalid number format for '%s' - non-numeric character found\n",
                asm->logger.file_name,
                AUTMK_EXTENSION,
                asm->logger.line_num,
                data_type);
            return DATA_ENCODE_ERR;
        }

        digit_found = true;
        result = result * 10 + (*cursor - '0');
        ++cursor;
    }

    if (!digit_found)
    {
        log_error(
            "%s%s: line %d: invalid number format for '%s' - non-numeric character found\n",
            asm->logger.file_name,
            AUTMK_EXTENSION,
            asm->logger.line_num,
            data_type);
        return DATA_ENCODE_ERR;
    }

    while (*cursor != '\0')
    {
        if (*cursor != ' ' && *cursor != '\t' && *cursor != '\n')
        {
            log_error(
                "%s%s: line %d: unexpected characters after numeric value\n",
                asm->logger.file_name,
                AUTMK_EXTENSION,
                asm->logger.line_num);
            return DATA_ENCODE_ERR;
        }

        ++cursor;
    }

    int num = result * sign;

    if ((num >= 0 && ((num >> WORD_SIZE) != 0)) ||
        (num < 0 && ((num >> WORD_SIZE) != -1)))
    {
        log_warning(
            "%s%s: line %d: value may be truncated — exceeds %d-bit limit\n",
            asm->logger.file_name,
            AUTMK_EXTENSION,
            asm->logger.line_num,
            WORD_SIZE);
    }

    DataWord *data_word = new_data_word((short) num);
    asm->data[asm->dc++] = data_word;

    return DATA_ENCODE_OK;
}

/**
 * @brief Encodes the payload of a .data, .string, or .struct directive.
 *
 * The function dispatches parsing according to @p data_type:
 * - .data   : tokenizes the buffer by commas and encodes each number
 * - .string : encodes a quoted character sequence plus a terminating '\0'
 * - .struct : encodes one numeric field followed by one string field
 *
 * The input buffer may be modified in place because tokenization is performed
 * with `strtok()`.
 *
 * @param asm Assembler context that owns the data image and logger state.
 * @param buf Mutable buffer containing the directive payload.
 * @param data_type Directive token that identifies the directive kind.
 * @return DATA_ENCODE_OK on success, or DATA_ENCODE_ERR on failure.
 */
DataEncodeStatus encode_dss(Assembler *asm, char *buf, char *data_type)
{
    char *token;

    if (!asm || !buf || !data_type)
        return DATA_ENCODE_ERR;

    if (is_data(data_type))
    {
        /*
         * Detect ",," early to report redundant separators before tokenization.
         * This preserves the original intent of the existing validation logic.
         */
        for (const char *cursor = buf; *cursor != '\n' && *cursor != '\0'; ++cursor)
        {
            if (*cursor == ',' && *(cursor + 1) == ',')
            {
                log_error(
                    "%s%s: line %d: invalid format in '%s' declaration — redundant comma\n",
                    asm->logger.file_name,
                    AUTMK_EXTENSION,
                    asm->logger.line_num,
                    data_type);
                return DATA_ENCODE_ERR;
            }
        }

        token = strtok(buf, ",");

        while (token)
        {
            if (encode_numeric_data(asm, token, data_type) == DATA_ENCODE_ERR)
                return DATA_ENCODE_ERR;

            token = strtok(NULL, ",");
        }
    }
    else if (is_string(data_type))
    {
        if (encode_string_data(asm, buf, data_type) == DATA_ENCODE_ERR)
            return DATA_ENCODE_ERR;
    }
    else if (is_struct(data_type))
    {
        token = strtok(buf, ",");

        if (token == NULL || encode_numeric_data(asm, token, data_type) == DATA_ENCODE_ERR)
            return DATA_ENCODE_ERR;

        token = strtok(NULL, "\0");
        if (token == NULL)
        {
            log_error(
                "%s%s: line %d: incomplete definition — '%s' is missing a field\n",
                asm->logger.file_name,
                AUTMK_EXTENSION,
                asm->logger.line_num,
                data_type);
            return DATA_ENCODE_ERR;
        }

        if (encode_string_data(asm, token, data_type) == DATA_ENCODE_ERR)
            return DATA_ENCODE_ERR;
    }

    return DATA_ENCODE_OK;
}
