/**
 * @file token_classifier.c
 * @brief Classifies assembler tokens such as directives, instructions, and registers.
 *
 * This module provides helper functions for recognizing reserved assembler
 * words, including:
 * - data/string/struct directives,
 * - entry/extern directives,
 * - instruction mnemonics,
 * - register names.
 *
 * It also provides a utility for checking whether a plain word matches a
 * directive name by prepending the directive prefix character '.'.
 *
 * @author Tehila Cahnaman
 */

#include "token_classifier.h"

#include <string.h>

#include "isa.h"
#include "macro_table.h"

/**
 * @brief Checks whether a token is one of the assembler directives.
 *
 * This function expects the directive in its full form, including the leading
 * dot character (for example, ".data").
 *
 * @param token Token to classify.
 * @return true if the token is a recognized directive, false otherwise.
 */
static bool is_directive(const char *token)
{
    return is_string(token) ||
           is_data(token) ||
           is_struct(token) ||
           is_entry(token) ||
           is_extern(token);
}

/**
 * @brief Checks whether a token is the .data directive.
 *
 * @param token Token to classify.
 * @return true if the token matches the data directive, false otherwise.
 */
bool is_data(const char *token)
{
    if (!token)
        return false;

    return strcmp(token, DATA_DIRECTIVE) == 0;
}

/**
 * @brief Checks whether a token is the .struct directive.
 *
 * @param token Token to classify.
 * @return true if the token matches the struct directive, false otherwise.
 */
bool is_struct(const char *token)
{
    if (!token)
        return false;

    return strcmp(token, STRUCT_DIRECTIVE) == 0;
}

/**
 * @brief Checks whether a token is the .string directive.
 *
 * @param token Token to classify.
 * @return true if the token matches the string directive, false otherwise.
 */
bool is_string(const char *token)
{
    if (!token)
        return false;

    return strcmp(token, STR_DIRECTIVE) == 0;
}

/**
 * @brief Checks whether a token is the .entry directive.
 *
 * @param token Token to classify.
 * @return true if the token matches the entry directive, false otherwise.
 */
bool is_entry(const char *token)
{
    if (!token)
        return false;

    return strcmp(token, ENTRY_DIRECTIVE) == 0;
}

/**
 * @brief Checks whether a token is the .extern directive.
 *
 * @param token Token to classify.
 * @return true if the token matches the extern directive, false otherwise.
 */
bool is_extern(const char *token)
{
    if (!token)
        return false;

    return strcmp(token, EXTERN_DIRECTIVE) == 0;
}

/**
 * @brief Checks whether a token is a valid instruction mnemonic.
 *
 * @param token Token to classify.
 * @return true if the token matches a known instruction mnemonic, false otherwise.
 */
bool is_instruction(const char *token)
{
    int i;

    if (!token)
        return false;

    for (i = 0; i < INSTR_SET; ++i)
    {
        if (strcmp(token, isa_get_instr_name(i)) == 0)
            return true;
    }

    return false;
}

/**
 * @brief Checks whether a token is a valid register name.
 *
 * @param token Token to classify.
 * @return true if the token matches a known register name, false otherwise.
 */
bool is_register(const char *token)
{
    int i;

    if (!token)
        return false;

    for (i = 0; i < REGISTERS_NUM; ++i)
    {
        if (strcmp(token, isa_get_register(i)) == 0)
            return true;
    }

    return false;
}

/**
 * @brief Checks whether a plain word matches a known directive name.
 *
 * This helper builds a temporary string by prepending '.' to the given word,
 * then checks whether the resulting token is a recognized directive.
 * For example, "data" becomes ".data".
 *
 * @param word Plain word to classify.
 * @return true if the word corresponds to a known directive, false otherwise.
 */
bool word_is_directive(const char *word)
{
    char name[MAX_LINE_LEN];
    int i = 0;

    if (!word)
        return false;

    name[i++] = '.';

    while (*word != '\0' && i < MAX_LINE_LEN - 1)
    {
        name[i] = *word;
        ++i;
        ++word;
    }

    name[i] = '\0';

    return is_directive(name);
}
