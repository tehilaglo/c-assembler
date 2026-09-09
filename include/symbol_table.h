/**
 * @file symbol_table.h
 * @brief Symbol table declarations for the assembler.
 *
 * @author Tehila Cahnaman
 */

#pragma once

#include <stdbool.h>

/**
 * @def MAX_LABEL_LEN
 * @brief Maximum allowed length for a symbol label.
 */
#define MAX_LABEL_LEN 30

/**
 * @enum SymbolType
 * @brief Represents the semantic type assigned to a symbol.
 */
typedef enum
{
    SYMBOL_CODE,  /**< Symbol marks a code label. */
    SYMBOL_DATA,  /**< Symbol marks a data label. */
    SYMBOL_EXT,   /**< Symbol is declared as external. */
    SYMBOL_ENTRY  /**< Symbol is marked as an entry. */
} SymbolType;

/**
 * @struct Symbol
 * @brief Represents one symbol node in the symbol table.
 *
 * Each symbol stores its label name, its address, a small group of bit-field
 * attributes describing its role, and a pointer to the next symbol in the list.
 */
typedef struct Symbol
{
    char name[MAX_LABEL_LEN + 1]; /**< Null-terminated symbol label. */
    unsigned char address;        /**< Symbol address in the assembled image. */

    struct
    {
        unsigned char code : 1;      /**< Non-zero if this is a code symbol. */
        unsigned char data : 1;      /**< Non-zero if this is a data symbol. */
        unsigned char external : 1;  /**< Non-zero if this is an external symbol. */
        unsigned char entry : 1;     /**< Non-zero if this is an entry symbol. */
    } attr;

    struct Symbol *next; /**< Next symbol in the singly linked list. */
} Symbol;

/**
 * @struct SymbolTable
 * @brief Wraps the head pointer of the symbol linked list.
 */
typedef struct SymbolTable
{
    Symbol *head; /**< First symbol in the table, or NULL when empty. */
} SymbolTable;

/**
 * @brief Allocates and initializes a new symbol node.
 *
 * @param symbol_name Name of the symbol to create.
 * @param type Semantic symbol type.
 * @param address Address associated with the symbol.
 * @return Pointer to the newly allocated symbol node.
 */
Symbol *new_symbol(char *symbol_name, SymbolType type, unsigned char address);

/**
 * @brief Searches for a symbol by name.
 *
 * @param symbols Symbol table to search.
 * @param symbol_name Symbol name to look up.
 * @return Matching symbol node, or NULL if not found.
 */
Symbol *get_symbol(const SymbolTable *symbols, const char *symbol_name);

/**
 * @brief Inserts a symbol node at the front of the symbol table.
 *
 * @param symbols Symbol table receiving the node.
 * @param node Symbol node to insert.
 */
void insert_symbol(SymbolTable *symbols, Symbol *node);

/**
 * @brief Frees all symbols in the table and resets the head pointer.
 *
 * @param symbols Symbol table to clear.
 */
void free_symbols(SymbolTable *symbols);

/**
 * @brief Checks whether a symbol with the given name exists.
 *
 * @param symbols Symbol table to search.
 * @param symbol_name Symbol name to look up.
 * @return true if the symbol exists, false otherwise.
 */
bool symbol_exist(const SymbolTable *symbols, const char *symbol_name);

/**
 * @brief Checks whether the table contains any external symbol.
 *
 * @param symbols Symbol table to inspect.
 * @return true if at least one external symbol exists, false otherwise.
 */
bool extern_exist(const SymbolTable *symbols);

/**
 * @brief Checks whether the table contains any entry symbol.
 *
 * @param symbols Symbol table to inspect.
 * @return true if at least one entry symbol exists, false otherwise.
 */
bool entry_exist(const SymbolTable *symbols);
