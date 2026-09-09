/**
 * @file assembler_pass1.c
 * @brief Implements the first pass of the assembler.
 *
 * The first pass scans the source file line by line, identifies labels,
 * classifies statements, inserts symbols into the symbol table, and encodes
 * instructions and data directives into the assembler's internal structures.
 *
 * @author Tehila Cahnaman
 */

#include "assembler_pass1.h"

#include <stdio.h>

#include "data_directive_encoder.h"
#include "file_config.h"
#include "instruction_encoder.h"
#include "label_parser.h"
#include "line_parser.h"
#include "macro_table.h"
#include "symbol_table.h"
#include "token_classifier.h"

/**
 * @enum Pass1Status
 * @brief Result codes used internally while processing statements in pass 1.
 */
typedef enum
{
    PASS1_OK = 0,               /**< Statement was processed successfully. */
    PASS1_ERROR,                /**< Statement was recognized but failed validation/encoding. */
    PASS1_UNKNOWN_STATEMENT     /**< Statement did not match any known form. */
} Pass1Status;

/**
 * @brief Inserts a symbol into the assembler symbol table.
 *
 * The symbol is inserted at the head of the list. If the table is empty,
 * the new symbol becomes the head node.
 *
 * @param asm Assembler state containing the symbol table.
 * @param name Symbol name to insert.
 * @param type Symbol classification.
 * @param address Symbol address value.
 * @return true if insertion completed successfully.
 */
static bool insert_symbol_if_needed(Assembler *asm,
                                    const char *name,
                                    const SymbolType type,
                                    const unsigned char address)
{
    Symbol *symbol = new_symbol((char *) name, type, address);

    if (asm->symbols.head == NULL)
        asm->symbols.head = symbol;
    else
        insert_symbol(&asm->symbols, symbol);

    return true;
}

/**
 * @brief Handles an `.extern` directive during pass 1.
 *
 * The operand is expected to contain the external symbol name. The symbol is
 * inserted into the symbol table with external attributes.
 *
 * @param asm Assembler state to update.
 * @param operand Remainder of the line after the directive keyword.
 * @return true on success, false on syntax error.
 */
static bool handle_extern_directive(Assembler *asm, char *operand)
{
    char symbol_name[MAX_LINE_LEN];

    if (operand == NULL)
        return false;

    skip_inline_ws(&operand);

    if (*operand == '\n' || *operand == '\0')
    {
        log_error("%s%s: line %d: directive '%s' missing operand (e.g., label name)\n",
                  asm->logger.file_name,
                  AUTMK_EXTENSION,
                  asm->logger.line_num,
                  EXTERN_DIRECTIVE);
        return false;
    }

    extract_word(symbol_name, operand);
    insert_symbol_if_needed(asm, symbol_name, SYMBOL_EXT, MAX_FREE_MEM);

    return true;
}

/**
 * @brief Processes an unlabeled statement.
 *
 * This function dispatches the line based on whether the first token is a
 * data directive, entry/extern directive, or instruction mnemonic.
 *
 * @param asm Current assembler state.
 * @param keyword First parsed token of the statement.
 * @param rest Remaining text after the first token.
 * @return Internal pass-1 status code.
 */
static Pass1Status process_statement(Assembler *asm, char *keyword, char *rest)
{
    if ( rest == NULL)
        return PASS1_ERROR;

    if (is_data(keyword) || is_string(keyword) || is_struct(keyword))
    {
        return (encode_dss(asm, rest, keyword) == DATA_ENCODE_OK)
                   ? PASS1_OK
                   : PASS1_ERROR;
    }

    if (is_extern(keyword))
    {
        return handle_extern_directive(asm, rest)
                   ? PASS1_OK
                   : PASS1_ERROR;
    }

    if (is_entry(keyword))
    {
        /* Entry symbols are resolved during pass 2. */
        return PASS1_OK;
    }

    if (is_instruction(keyword))
    {
        return encode_instruction(asm, rest, keyword)
                   ? PASS1_OK
                   : PASS1_ERROR;
    }

    return PASS1_UNKNOWN_STATEMENT;
}

/**
 * @brief Processes a statement that begins with a valid label definition.
 *
 * Depending on the statement type that follows the label, the label is added
 * as either a data symbol or a code symbol before encoding continues.
 *
 * @param asm Current assembler state.
 * @param label_name Parsed label name.
 * @param keyword First token after the label.
 * @param rest Remaining text after the keyword.
 * @return Internal pass-1 status code.
 */
static Pass1Status process_labeled_statement(Assembler *asm,
                                             const char *label_name,
                                             char *keyword,
                                             char *rest)
{
    if (rest == NULL)
        return PASS1_ERROR;

    if (is_data(keyword) || is_string(keyword) || is_struct(keyword))
    {
        insert_symbol_if_needed(asm, label_name, SYMBOL_DATA, (unsigned char)asm->dc);

        return (encode_dss(asm, rest, keyword) == DATA_ENCODE_OK)
                   ? PASS1_OK
                   : PASS1_ERROR;
    }

    if (is_instruction(keyword))
    {
        insert_symbol_if_needed(asm, label_name, SYMBOL_CODE, (unsigned char)asm->ic);

        return encode_instruction(asm, rest, keyword)
                   ? PASS1_OK
                   : PASS1_ERROR;
    }

    if (is_extern(keyword) || is_entry(keyword))
    {
        log_warning("%s%s: line %d: label '%s' before '%s' is redundant\n",
                    asm->logger.file_name,
                    AUTMK_EXTENSION,
                    asm->logger.line_num,
                    label_name,
                    keyword);

        if (is_extern(keyword))
        {
            return handle_extern_directive(asm, rest)
                       ? PASS1_OK
                       : PASS1_ERROR;
        }

        return PASS1_OK;
    }

    log_error("%s%s: line %d: '%s' is not a valid directive or instruction\n",
              asm->logger.file_name,
              AUTMK_EXTENSION,
              asm->logger.line_num,
              keyword);

    return PASS1_ERROR;
}

/**
 * @brief Processes one logical source line during pass 1.
 *
 * The line is first stripped of leading whitespace/comments, then checked for
 * an optional label definition. Processing is delegated to either labeled or
 * unlabeled statement handlers.
 *
 * @param asm Current assembler state.
 * @param line Source line buffer.
 * @return true if the line was processed successfully, false otherwise.
 */
static bool process_line(Assembler *asm, char *line)
{
    char first_word[MAX_LINE_LEN];
    char keyword[MAX_LINE_LEN];
    char label_name[MAX_LINE_LEN];
    char *cursor;
    Pass1Status status;

    cursor = line;
    cursor = skip_ws_and_comments(&cursor);

    if (cursor == NULL)
        return true; /* Empty line or comment line. */

    cursor = extract_word(first_word, cursor);
    LabelCheckResult label_status = parse_label_definition(asm, first_word, label_name);

    if (label_status == LABEL_INVALID)
        return false;

    if (label_status == LABEL_VALID)
    {
        skip_inline_ws(&cursor);

        if (*cursor == '\n' || *cursor == '\0')
        {
            log_error("%s%s: line %d: label '%s' has no data/instruction\n",
                      asm->logger.file_name,
                      AUTMK_EXTENSION,
                      asm->logger.line_num,
                      label_name);
            return false;
        }

        cursor = extract_word(keyword, cursor);
        status = process_labeled_statement(asm, label_name, keyword, cursor);
    }
    else
    {
        status = process_statement(asm, first_word, cursor);
    }

    if (status == PASS1_UNKNOWN_STATEMENT)
    {
        log_error("%s%s: line %d: unrecognized command '%s'\n",
                  asm->logger.file_name,
                  AUTMK_EXTENSION,
                  asm->logger.line_num,
                  first_word);
        return false;
    }

    return status == PASS1_OK;
}

/**
 * @brief Executes the first pass over the input file.
 *
 * The file is read line by line, line numbers are updated for diagnostics,
 * and each line is processed independently. The function returns false if
 * any line caused an error during the pass.
 *
 * @param fp Input file stream.
 * @param asm Assembler state object to update.
 * @return true if pass 1 completed successfully, false otherwise.
 */
bool assembler_pass1(FILE *fp, Assembler *asm)
{
    char line[MAX_LINE_LEN + 1];
    bool has_error = false;

    if (fp == NULL || asm == NULL)
        return false;

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        ++asm->logger.line_num;

        if (!process_line(asm, line))
            has_error = true;
    }

    return !has_error;
}
