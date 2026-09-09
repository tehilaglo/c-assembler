/**
 * @file label_parser.c
 * @brief Parses and validates label definitions in assembler source lines.
 *
 * This module is responsible for:
 * - detecting whether a token defines a label,
 * - extracting the label name from a token ending with ':',
 * - validating label naming rules,
 * - checking for reserved-word conflicts, and
 * - detecting duplicate symbol definitions.
 *
 * @author Tehila Cahnaman
 */

#include "label_parser.h"

#include <string.h>

#include "assembler.h"
#include "file_config.h"
#include "logger.h"
#include "symbol_table.h"
#include "token_classifier.h"

/**
 * @brief Extracts a label name from a token ending with ':'.
 *
 * The function checks whether the given token ends with a colon. If so,
 * all characters before the colon are copied into @p symbol_name.
 *
 * @param buf Input token to inspect.
 * @param symbol_name Output buffer receiving the extracted label name.
 * @return true if the token is a label definition, false otherwise.
 */
static bool extract_label_name(const char *buf, char *symbol_name)
{
    size_t i;

    const size_t len = strlen(buf);

    if (len == 0 || buf[len - 1] != ':')
        return false;

    /* Copy all characters before the trailing ':' into the destination buffer. */
    for (i = 0; buf[i] != ':'; ++i)
        symbol_name[i] = buf[i];

    symbol_name[i] = '\0';
    return true;
}

/**
 * @brief Validates a parsed label name.
 *
 * Validation includes:
 * - maximum length,
 * - first character must be alphabetic,
 * - remaining characters must be alphanumeric,
 * - must not conflict with instruction names,
 * - must not conflict with register names,
 * - must not conflict with directive keywords.
 *
 * @param asm Assembler context used for diagnostics.
 * @param symbol_name Label name to validate.
 * @return true if the label name is valid, false otherwise.
 */
static bool validate_label_name(const Assembler *asm, const char *symbol_name)
{
    size_t i;

    if (strlen(symbol_name) > MAX_LABEL_LEN)
    {
        log_error(
            "%s%s: line %d: label name exceeds maximum allowed length of %d characters\n",
            asm->logger.file_name,
            AUTMK_EXTENSION,
            asm->logger.line_num,
            MAX_LABEL_LEN);
        return false;
    }

    if ((symbol_name[0] < 'A' || symbol_name[0] > 'Z') &&
        (symbol_name[0] < 'a' || symbol_name[0] > 'z'))
    {
        log_error(
            "%s%s: line %d: label name must start with an alphabetic character\n",
            asm->logger.file_name,
            AUTMK_EXTENSION,
            asm->logger.line_num);
        return false;
    }

    for (i = 1; symbol_name[i] != '\0'; ++i)
    {
        if ((symbol_name[i] < 'A' || symbol_name[i] > 'Z') &&
            (symbol_name[i] < 'a' || symbol_name[i] > 'z') &&
            (symbol_name[i] < '0' || symbol_name[i] > '9'))
        {
            log_error(
                "%s%s: line %d: label name must contain only alphanumeric characters\n",
                asm->logger.file_name,
                AUTMK_EXTENSION,
                asm->logger.line_num);
            return false;
        }
    }

    if (is_instruction(symbol_name))
    {
        log_error(
            "%s%s: line %d: label name '%s' conflicts with an instruction mnemonic\n",
            asm->logger.file_name,
            AUTMK_EXTENSION,
            asm->logger.line_num,
            symbol_name);
        return false;
    }

    if (is_register(symbol_name))
    {
        log_error(
            "%s%s: line %d: invalid label name '%s': name is reserved for registers\n",
            asm->logger.file_name,
            AUTMK_EXTENSION,
            asm->logger.line_num,
            symbol_name);
        return false;
    }

    if (word_is_directive(symbol_name))
    {
        log_error(
            "%s%s: line %d: label name '%s' conflicts with a directive keyword\n",
            asm->logger.file_name,
            AUTMK_EXTENSION,
            asm->logger.line_num,
            symbol_name);
        return false;
    }

    return true;
}

/**
 * @brief Parses and validates a label definition token.
 *
 * If the input token does not define a label, LABEL_NOT_PRESENT is returned.
 * If the token defines a label but violates syntax or naming rules,
 * LABEL_INVALID is returned. Otherwise, LABEL_VALID is returned.
 *
 * The function also checks for duplicate label definitions in the symbol table.
 *
 * @param asm Assembler context used for diagnostics and duplicate checks.
 * @param buf Input token to inspect.
 * @param symbol_name Output buffer receiving the extracted label name.
 * @return LABEL_VALID, LABEL_NOT_PRESENT, or LABEL_INVALID.
 */
LabelCheckResult parse_label_definition(const Assembler *asm,
                                        const char *buf,
                                        char *symbol_name)
{
    if (!asm || !buf || !symbol_name)
        return LABEL_INVALID;

    if (!extract_label_name(buf, symbol_name))
        return LABEL_NOT_PRESENT;

    if (!validate_label_name(asm, symbol_name))
        return LABEL_INVALID;

    if (!asm->symbols.head)
        return LABEL_VALID;

    if (symbol_exist(&asm->symbols, symbol_name))
    {
        log_error(
            "%s%s: line %d: duplicate label definition: '%s' was previously declared\n",
            asm->logger.file_name,
            AUTMK_EXTENSION,
            asm->logger.line_num,
            symbol_name);
        return LABEL_INVALID;
    }

    return LABEL_VALID;
}
