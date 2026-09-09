/**
 * @file operand_parser.c
 * @brief Parses instruction operands for the assembler.
 *
 * This module provides parsing helpers for the assembler's supported operand
 * addressing modes:
 * - immediate operands (e.g. #5, #-3)
 * - struct operands (e.g. LABEL.1)
 * - register operands (e.g. r3)
 * - direct operands (e.g. LABEL)
 *
 * Each parser returns an OperandParseResult indicating whether:
 * - the operand matched and was parsed successfully,
 * - the operand did not match the expected addressing mode, or
 * - the operand matched syntactically but contains an error.
 *
 * @author Tehila Cahnaman
 */

#include "operand_parser.h"

#include <ctype.h>
#include <string.h>

#include "file_config.h"
#include "line_parser.h"
#include "logger.h"
#include "macro_table.h"
#include "token_classifier.h"

/**
 * @brief Checks whether a character terminates an operand token.
 * @param ch Character to test.
 * @return true if the character is a token terminator, false otherwise.
 */
static bool is_operand_terminator(const char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\0' || ch == '\n';
}

/**
 * @brief Parses an immediate operand.
 *
 * Expected syntax: #<signed-number>
 *
 * Leading inline whitespace is skipped before parsing. If the operand does not
 * begin with '#', this parser does not apply and OPERAND_NOT_MATCH is returned.
 *
 * @param asm Assembler context used for diagnostics.
 * @param cursor Pointer to the operand text.
 * @param instr Instruction mnemonic for error reporting.
 * @param num Output parameter for the parsed numeric value.
 * @return OPERAND_OK on success, OPERAND_NOT_MATCH if this is not an immediate
 *         operand, or OPERAND_ERR if the syntax is invalid.
 */
OperandParseResult parse_immediate_operand(const Assembler *asm,
                                           char *cursor,
                                           char *instr,
                                           int *num)
{
    int sign = 1;
    int result = 0;
    bool has_error = false;
    bool digit_found = false;

    if (!asm || !cursor || !instr || !num)
        return OPERAND_ERR;

    skip_inline_ws(&cursor);

    if (*cursor != '#')
        return OPERAND_NOT_MATCH;

    ++cursor;

    if (*cursor == '-')
    {
        sign = -1;
        ++cursor;
    }
    else if (*cursor == '+')
    {
        ++cursor;
    }

    while (!is_operand_terminator(*cursor))
    {
        if (*cursor < '0' || *cursor > '9')
        {
            if (!has_error)
            {
                log_error(
                    "%s%s: line %d: immediate operand in '%s' is not a valid numeric value\n",
                    asm->logger.file_name,
                    AUTMK_EXTENSION,
                    asm->logger.line_num,
                    instr);
                has_error = true;
            }
        }

        digit_found = true;

        /* Build the integer value digit by digit from its ASCII representation. */
        result = result * 10 + (*cursor - '0');
        ++cursor;
    }

    if (!digit_found)
    {
        log_error(
            "%s%s: line %d: syntax error: expected a number after '#' in '%s'\n",
            asm->logger.file_name,
            AUTMK_EXTENSION,
            asm->logger.line_num,
            instr);
        has_error = true;
    }

    if (has_error)
        return OPERAND_ERR;

    *num = result * sign;
    return OPERAND_OK;
}

/**
 * @brief Parses a struct operand.
 *
 * Expected syntax: <symbol>.<field>
 * Supported field values are 1 and 2.
 *
 * Leading inline whitespace is skipped before parsing. The function splits the
 * input buffer in place at the '.' character when present.
 *
 * @param asm Assembler context used for diagnostics.
 * @param buf Mutable operand buffer.
 * @param instr Instruction mnemonic for error reporting.
 * @param symbol Output buffer that receives the symbol name.
 * @param field Output parameter that receives the struct field number.
 * @return OPERAND_OK on success, OPERAND_NOT_MATCH if this is not a struct
 *         operand, or OPERAND_ERR if the syntax is invalid.
 */
OperandParseResult parse_struct_operand(const Assembler *asm,
                                        char *buf,
                                        char *instr,
                                        char *symbol,
                                        int *field)
{
    bool has_dot = false;

    if (!asm || !buf || !instr || !symbol || !field)
        return OPERAND_ERR;

    skip_inline_ws(&buf);
    char *cursor = buf;

    while (*cursor != '\0')
    {
        if (*cursor == '.')
        {
            *cursor = '\0';
            ++cursor;
            has_dot = true;
            break;
        }
        ++cursor;
    }

    /* No dot means this operand is not using struct addressing. */
    if (!has_dot)
        return OPERAND_NOT_MATCH;

    snprintf(symbol, MAX_LINE_LEN - MAX_INSTR_LEN, "%s", buf);

    if (*cursor == '\0' || *cursor == '\n')
    {
        log_error(
            "%s%s: line %d: structure operand in '%s' lacks a field specifier (e.g., '.1')\n",
            asm->logger.file_name,
            AUTMK_EXTENSION,
            asm->logger.line_num,
            instr);
        return OPERAND_ERR;
    }

    if (*cursor != '1' && *cursor != '2')
    {
        log_error(
            "%s%s: line %d: '%s' specifies a structure with an unrecognized field; expected '.1' or '.2'\n",
            asm->logger.file_name,
            AUTMK_EXTENSION,
            asm->logger.line_num,
            instr);
        return OPERAND_ERR;
    }

    *field = (*cursor == '1') ? 1 : 2;
    return OPERAND_OK;
}

/**
 * @brief Parses a register operand.
 *
 * Expected syntax: r0 through r7.
 *
 * Leading inline whitespace is skipped before parsing.
 *
 * @param cursor Pointer to the operand text.
 * @param reg_num Output parameter for the parsed register number.
 * @return OPERAND_OK on success, OPERAND_NOT_MATCH if the operand is not a
 *         valid register, or OPERAND_ERR on invalid arguments.
 */
OperandParseResult parse_register_operand(char *cursor, int *reg_num)
{
    char reg[MAX_LINE_LEN - 2];
    int i = 0;

    if (!cursor || !reg_num)
        return OPERAND_ERR;

    skip_inline_ws(&cursor);

    while (!is_operand_terminator(*cursor))
    {
        reg[i] = *cursor;

        if (*cursor >= '0' && *cursor <= '7')
            *reg_num = *cursor - '0';

        ++i;
        ++cursor;
    }

    reg[i] = '\0';

    if (!is_register(reg))
        return OPERAND_NOT_MATCH;

    return OPERAND_OK;
}

/**
 * @brief Parses a direct operand.
 *
 * Expected syntax: a single label-like token containing only alphanumeric
 * characters. The parsed symbol must not conflict with directives,
 * instruction mnemonics, or register names.
 *
 * Leading inline whitespace is skipped before parsing. The input buffer may be
 * modified in place by inserting a string terminator after the operand token.
 *
 * @param asm Assembler context used for diagnostics.
 * @param buf Mutable operand buffer.
 * @param instr Instruction mnemonic for error reporting.
 * @param symbol Output buffer that receives the parsed symbol.
 * @return OPERAND_OK on success, OPERAND_ERR if the operand is invalid.
 */
OperandParseResult parse_direct_operand(const Assembler *asm,
                                        char *buf,
                                        char *instr,
                                        char *symbol)
{
    char *cursor;

    if (!asm || !buf || !instr || !symbol)
        return OPERAND_ERR;

    skip_inline_ws(&buf);
    cursor = buf;

    if (*buf == '\0' || *buf == '\n')
    {
        log_error(
            "%s%s: line %d: syntax error: operand missing for '%s'\n",
            asm->logger.file_name,
            AUTMK_EXTENSION,
            asm->logger.line_num,
            instr);
        return OPERAND_ERR;
    }

    while (*cursor != ' ' && *cursor != '\t' && *cursor != '\n' && *cursor != '\0')
    {
        if (!isalnum((unsigned char)*cursor))
        {
            log_error(
                "%s%s: line %d: invalid label in '%s'; labels must contain only alphanumeric characters\n",
                asm->logger.file_name,
                AUTMK_EXTENSION,
                asm->logger.line_num,
                instr);
            return OPERAND_ERR;
        }
        ++cursor;
    }

    skip_inline_ws(&cursor);

    if (*cursor != '\0' && *cursor != '\n')
    {
        log_error(
            "%s%s: line %d: invalid label in '%s'; labels must contain only alphanumeric characters\n",
            asm->logger.file_name,
            AUTMK_EXTENSION,
            asm->logger.line_num,
            instr);
        return OPERAND_ERR;
    }

    *cursor = '\0';

    if (word_is_directive(buf))
    {
        log_error(
            "%s%s: line %d: label reference in '%s' conflicts with a directive keyword\n",
            asm->logger.file_name,
            AUTMK_EXTENSION,
            asm->logger.line_num,
            instr);
        return OPERAND_ERR;
    }

    if (is_instruction(buf))
    {
        log_error(
            "%s%s: line %d: label reference in '%s' conflicts with an instruction mnemonic\n",
            asm->logger.file_name,
            AUTMK_EXTENSION,
            asm->logger.line_num,
            instr);
        return OPERAND_ERR;
    }

    if (is_register(buf))
    {
        log_error(
            "%s%s: line %d: label reference in '%s' conflicts with a register name\n",
            asm->logger.file_name,
            AUTMK_EXTENSION,
            asm->logger.line_num,
            instr);
        return OPERAND_ERR;
    }

    snprintf(symbol, MAX_LINE_LEN - MAX_INSTR_LEN, "%s", buf);
    return OPERAND_OK;
}
