/**
 * @file macro_table.h
 * @brief Declarations for the assembler macro linked-list table.
 *
 * @author Tehila Cahnaman
 */

#pragma once

#include <stdbool.h>

/**
 * @def MAX_MACRO_NAME
 * @brief Maximum length allowed for a macro identifier.
 */
#define MAX_MACRO_NAME 74
/**
 * @def MAX_LINE_LEN
 * @brief Maximum length of a single source line in the assembler input.
 */
#define MAX_LINE_LEN 80

/**
 * @struct Macro
 * @brief Represents a single macro definition in the macro table.
 *
 * This structure stores:
 * - the macro name,
 * - the start offset of its body in the source file,
 * - the end offset of its body in the source file,
 * - a pointer to the next macro in the table.
 */
typedef struct Macro
{
    char name[MAX_MACRO_NAME + 1]; /**< Macro identifier. */

    long int start_offset; /**< File offset where the macro body begins. */
    long int end_offset;   /**< File offset where the macro body ends. */

    struct Macro *next; /**< Next macro in the linked list. */
} Macro;

/**
 * @struct MacroTable
 * @brief Wraps the head of the macro linked list.
 */
typedef struct MacroTable
{
    Macro *head; /**< Head node of the macro list. */
} MacroTable;

/**
 * @brief Allocates and initializes a new macro node.
 *
 * @param macro_name Name of the macro to create.
 * @param start_offset File offset where the macro body begins.
 * @param end_offset File offset where the macro body ends.
 * @return Pointer to the newly allocated macro node.
 */
Macro *new_macro(char *macro_name, long int start_offset, long int end_offset);

/**
 * @brief Checks whether a macro with the given name exists in the table.
 *
 * @param macros Macro table to search.
 * @param macro_name Macro name to look up.
 * @return true if a matching macro exists, false otherwise.
 */
bool macro_exists(const MacroTable *macros, const char *macro_name);

/**
 * @brief Inserts a macro node at the beginning of the list after the head node.
 *
 * @param macros Macro table receiving the new node.
 * @param node Macro node to insert.
 */
void insert_macro(const MacroTable *macros, Macro *node);

/**
 * @brief Parses a macro name from a cursor and returns the matching node.
 *
 * @param macros Macro table to search.
 * @param cursor Cursor pointing into a source line.
 * @return Matching macro node, or NULL if no valid single-name match exists.
 */
Macro *get_macro(const MacroTable *macros, char *cursor);

/**
 * @brief Frees all macro nodes in the table.
 *
 * @param macros Macro table to clear.
 */
void free_macros(MacroTable *macros);
