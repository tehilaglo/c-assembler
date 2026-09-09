/**
 * @file symbol_table.c
 * @brief Linked-list operations for the assembler symbol table.
 *
 * This module implements creation, insertion, lookup, existence checks,
 * and cleanup for assembler symbols. Symbols are stored in a singly linked
 * list wrapped by a SymbolTable structure.
 *
 * @author Tehila Cahnaman
 */

#include "symbol_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"

/**
 * @brief Resets all symbol attribute flags to zero.
 *
 * @param symbol Symbol node whose attributes will be cleared.
 */
static void clear_symbol_attributes(Symbol *symbol)
{
    if (symbol == NULL)
        return;

    symbol->attr.code = 0;
    symbol->attr.data = 0;
    symbol->attr.external = 0;
    symbol->attr.entry = 0;
}

/**
 * @brief Applies the requested semantic type flags to a symbol.
 *
 * Exactly one primary flag is enabled according to @p type.
 *
 * @param symbol Symbol node to update.
 * @param type Symbol classification to assign.
 */
static void set_symbol_type(Symbol *symbol, const SymbolType type)
{
    clear_symbol_attributes(symbol);

    switch (type)
    {
        case SYMBOL_CODE:
            symbol->attr.code = 1;
            break;

        case SYMBOL_DATA:
            symbol->attr.data = 1;
            break;

        case SYMBOL_EXT:
            symbol->attr.external = 1;
            break;

        case SYMBOL_ENTRY:
            symbol->attr.entry = 1;
            break;

        default:
            fatal_error("symbol type is not supported");
    }
}

/**
 * @brief Allocates and initializes a new symbol node.
 *
 * The function validates the symbol name, allocates memory for the node,
 * assigns its type flags, copies the name into the internal buffer, and
 * initializes the address and next pointer.
 *
 * @param symbol_name Name of the symbol to create.
 * @param type Semantic symbol type.
 * @param address Address associated with the symbol.
 * @return Pointer to the newly allocated symbol node.
 */
Symbol *new_symbol(char *symbol_name,
                   const SymbolType type,
                   const unsigned char address)
{
    if (symbol_name == NULL)
        fatal_error("symbol creation failed: symbol name is NULL");

    Symbol *symbol_node = malloc(sizeof(Symbol));
    if (symbol_node == NULL)
        fatal_error("memory allocation failed while creating symbol '%s'", symbol_name);

    set_symbol_type(symbol_node, type);

    snprintf(symbol_node->name, sizeof(symbol_node->name), "%s", symbol_name);
    symbol_node->address = address;
    symbol_node->next = NULL;

    return symbol_node;
}

/**
 * @brief Searches for a symbol by name.
 *
 * @param symbols Symbol table to search.
 * @param symbol_name Symbol name to look up.
 * @return Matching symbol node, or NULL if not found.
 */
Symbol *get_symbol(const SymbolTable *symbols, const char *symbol_name)
{
    if (symbols == NULL || symbol_name == NULL)
        return NULL;

    Symbol *current = symbols->head;

    while (current != NULL)
    {
        if (strcmp(symbol_name, current->name) == 0)
            return current;

        current = current->next;
    }

    return NULL;
}

/**
 * @brief Inserts a symbol node at the front of the symbol table.
 *
 * This function supports both empty and non-empty tables.
 *
 * @param symbols Symbol table receiving the node.
 * @param node Symbol node to insert.
 */
void insert_symbol(SymbolTable *symbols, Symbol *node)
{
    if (symbols == NULL || node == NULL)
        fatal_error("failed to insert symbol: symbol table or node is NULL");

    node->next = symbols->head;
    symbols->head = node;
}

/**
 * @brief Frees all symbols in the table and resets the head pointer.
 *
 * @param symbols Symbol table to clear.
 */
void free_symbols(SymbolTable *symbols)
{
    if (symbols == NULL)
        return;

    Symbol *current = symbols->head;

    while (current != NULL)
    {
        Symbol *next = current->next;
        free(current);
        current = next;
    }

    symbols->head = NULL;
}

/**
 * @brief Checks whether a symbol with the given name exists.
 *
 * @param symbols Symbol table to search.
 * @param symbol_name Symbol name to look up.
 * @return true if the symbol exists, false otherwise.
 */
bool symbol_exist(const SymbolTable *symbols, const char *symbol_name)
{
    if (symbols == NULL || symbol_name == NULL)
        return false;

    const Symbol *current = symbols->head;

    while (current != NULL)
    {
        if (strcmp(symbol_name, current->name) == 0)
            return true;

        current = current->next;
    }

    return false;
}

/**
 * @brief Checks whether the table contains any external symbol.
 *
 * @param symbols Symbol table to inspect.
 * @return true if at least one external symbol exists, false otherwise.
 */
bool extern_exist(const SymbolTable *symbols)
{
    if (symbols == NULL)
        return false;

    const Symbol *current = symbols->head;

    while (current != NULL)
    {
        if (current->attr.external)
            return true;

        current = current->next;
    }

    return false;
}

/**
 * @brief Checks whether the table contains any entry symbol.
 *
 * @param symbols Symbol table to inspect.
 * @return true if at least one entry symbol exists, false otherwise.
 */
bool entry_exist(const SymbolTable *symbols)
{
    if (symbols == NULL)
        return false;

    const Symbol *current = symbols->head;

    while (current != NULL)
    {
        if (current->attr.entry)
            return true;

        current = current->next;
    }

    return false;
}
