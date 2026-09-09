/**
 * @file macro_table.c
 * @brief Linked-list operations for the assembler macro table.
 *
 * This module implements creation, insertion, lookup, and cleanup for the
 * macro table used by the assembler preprocessor. Macros are stored in a
 * singly linked list and identified by name.
 *
 * @author Tehila Cahnaman
 */

#include "macro_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "line_parser.h"
#include "logger.h"

/**
 * @brief Allocates and initializes a new macro node.
 *
 * The function copies the provided macro name into the node's internal buffer,
 * stores the given file offsets, and initializes the next pointer to NULL.
 *
 * @param macro_name Name of the macro to create.
 * @param start_offset File offset where the macro body begins.
 * @param end_offset File offset where the macro body ends.
 * @return Pointer to the newly allocated macro node.
 */
Macro *new_macro(char *macro_name,
                 long int start_offset,
                 long int end_offset)
{
    if (macro_name == NULL)
    {
        fatal_error("failed to create macro — macro name must not be NULL");
    }

    Macro *macro_node = malloc(sizeof(Macro));
    if (macro_node == NULL)
    {
        fatal_error("memory allocation failed while creating macro '%s'", macro_name);
    }

    snprintf(macro_node->name, sizeof(macro_node->name), "%s", macro_name);
    macro_node->start_offset = start_offset;
    macro_node->end_offset = end_offset;
    macro_node->next = NULL;

    return macro_node;
}

/**
 * @brief Inserts a macro node immediately after the table head.
 *
 * This preserves the list structure while keeping insertion O(1).
 * The function expects both the table and the node to be valid, and also
 * requires the table head node to exist.
 *
 * @param macros Macro table receiving the new node.
 * @param node Macro node to insert.
 */
void insert_macro(const MacroTable *macros, Macro *node)
{
    if (macros == NULL || macros->head == NULL || node == NULL)
    {
        fatal_error("macro insertion failed — list head or macro node is undefined");
    }

    node->next = macros->head->next;
    macros->head->next = node;
}

/**
 * @brief Checks whether a macro with the given name exists in the table.
 *
 * @param macros Macro table to search.
 * @param macro_name Macro name to look up.
 * @return true if a matching macro exists, false otherwise.
 */
bool macro_exists(const MacroTable *macros, const char *macro_name)
{
    if (macros == NULL || macros->head == NULL || macro_name == NULL)
    {
        return false;
    }

    const Macro *current = macros->head;

    while (current != NULL)
    {
        if (strcmp(macro_name, current->name) == 0)
        {
            return true;
        }

        current = current->next;
    }

    return false;
}

/**
 * @brief Parses a macro name from a cursor and returns the matching node.
 *
 * Leading inline whitespace is skipped before parsing. The function extracts
 * one whitespace-delimited token as the macro name, then verifies that no
 * additional non-whitespace text appears afterward.
 *
 * @param macros Macro table to search.
 * @param cursor Cursor pointing into a source line.
 * @return Matching macro node, or NULL if the input is empty, malformed,
 *         or does not match any macro in the table.
 */
Macro *get_macro(const MacroTable *macros, char *cursor)
{
    char macro_name[MAX_LINE_LEN + 1];
    int i = 0;

    if (macros == NULL || macros->head == NULL || cursor == NULL)
    {
        return NULL;
    }

    skip_inline_ws(&cursor);

    /* No macro name is present. */
    if (*cursor == '\n' || *cursor == '\0')
    {
        return NULL;
    }

    /*
     * Extract a single word as the macro name.
     * Stop at inline whitespace, newline, or string terminator.
     */
    while (*cursor != ' ' && *cursor != '\t' &&
           *cursor != '\n' && *cursor != '\0')
    {
        if (i < MAX_LINE_LEN)
        {
            macro_name[i++] = *cursor;
        }
        ++cursor;
    }
    macro_name[i] = '\0';

    /* Reject trailing non-whitespace text after the macro name. */
    while (*cursor != '\0')
    {
        if (*cursor != ' ' && *cursor != '\t' && *cursor != '\n')
        {
            return NULL;
        }
        ++cursor;
    }

    Macro *current = macros->head;

    while (current != NULL)
    {
        if (strcmp(macro_name, current->name) == 0)
        {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

/**
 * @brief Frees all macro nodes in the table.
 *
 * The list is released node by node from the head forward. After cleanup,
 * the table head is set to NULL.
 *
 * @param macros Macro table to clear.
 */
void free_macros(MacroTable *macros)
{
    Macro *next;

    if (macros == NULL)
    {
        return;
    }

    Macro *current = macros->head;

    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }

    macros->head = NULL;
}
