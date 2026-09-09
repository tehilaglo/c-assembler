/**
 * @file line_parser.h
 * @brief Declarations for low-level line parsing helpers.
 *
 * @author Tehila Cahnaman
 */

#pragma once

#include <stdbool.h>

/**
 * @brief Extracts the next whitespace-delimited word from a source string.
 *
 * @param dest Destination buffer that receives the extracted word.
 * @param src Source string to read from.
 * @return Pointer to the stopping position in the source string.
 */
char *extract_word(char *dest, char *src);

/**
 * @brief Skips leading spaces and tabs and ignores blank/comment lines.
 *
 * @param p Address of the caller's cursor pointer.
 * @return Pointer to the first meaningful character, or NULL if the line is
 *         blank, a comment, or empty.
 */
char *skip_ws_and_comments(char **p);

/**
 * @brief Returns true if a character is an inline whitespace character.
 *
 * @param ch Character to test.
 * @return true if the character is a space or tab, false otherwise.
 */
bool is_inline_ws(const char ch);

/**
 * @brief Skips inline spaces and tabs.
 *
 * @param p Address of the caller's cursor pointer.
 */
void skip_inline_ws(char **p);
