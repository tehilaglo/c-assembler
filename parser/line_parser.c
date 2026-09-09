/**
 * @file line_parser.c
 * @brief Utility helpers for parsing words and skipping inline whitespace.
 *
 * This module provides small parsing helpers used while scanning assembler
 * source lines. The functions support:
 * - skipping leading inline whitespace,
 * - ignoring blank lines and comment lines,
 * - extracting the next whitespace-delimited word from a line.
 *
 * @author Tehila Cahnaman
 */

#include "line_parser.h"

#include <stddef.h>

/**
 * @brief Skips leading spaces and tabs and ignores blank/comment lines.
 *
 * The caller provides a pointer to a cursor pointer. The cursor is advanced
 * past any leading spaces or tabs. If the resulting position points to a blank
 * line, comment line, or end of string, NULL is returned. Otherwise, the
 * updated cursor is returned.
 *
 * @param p Address of the caller's cursor pointer.
 * @return Pointer to the first meaningful character, or NULL if the line is
 *         blank, a comment, or empty.
 */
char *skip_ws_and_comments(char **p)
{
    if (!p || !*p)
        return NULL;

    while (**p == ' ' || **p == '\t')
        ++(*p);

    return (**p == '\n' || **p == ';' || **p == '\0') ? NULL : *p;
}

/**
 * @brief Extracts the next whitespace-delimited word from a source string.
 *
 * Characters are copied from @p src into @p dest until a space, tab, newline,
 * or string terminator is reached. The destination string is always
 * null-terminated. The returned pointer indicates where parsing stopped in the
 * source string.
 *
 * @param dest Destination buffer that receives the extracted word.
 * @param src Source string to read from.
 * @return Pointer to the stopping position in the source string.
 */
char *extract_word(char *dest, char *src)
{
    int i = 0;

    if (!dest || !src)
        return src;

    while (*src != ' ' && *src != '\t' && *src != '\n' && *src != '\0')
    {
        dest[i] = *src;
        ++i;
        ++src;
    }

    dest[i] = '\0';
    return src;
}

/**
 * @brief Returns true if a character is an inline whitespace character.
 *
 * @param ch Character to test.
 * @return true if the character is a space or tab, false otherwise.
 */
bool is_inline_ws(const char ch)
{
    return ch == ' ' || ch == '\t';
}

/**
 * @brief Skips inline spaces and tabs.
 *
 * The caller provides a pointer to a cursor pointer. The cursor is advanced
 * in place until it points to a non-space, non-tab character.
 *
 * @param p Address of the caller's cursor pointer.
 */
void skip_inline_ws(char **p)
{
    if (!p || !*p)
        return;

    while (**p == ' ' || **p == '\t')
        ++(*p);
}
