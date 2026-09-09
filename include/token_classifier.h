/**
 * @file token_classifier.h
 * @brief Declarations for assembler token classification helpers.
 *
 * @author Tehila Cahnaman
 */

#pragma once

#include <stdbool.h>

/**
 * @brief Checks whether a plain word corresponds to a known directive.
 *
 * @param word Plain word to classify.
 * @return true if the word corresponds to a directive, false otherwise.
 */
bool word_is_directive(const char *word);

/**
 * @brief Checks whether a token is a valid instruction mnemonic.
 *
 * @param token Token to classify.
 * @return true if the token is a recognized instruction, false otherwise.
 */
bool is_instruction(const char *token);

/**
 * @brief Checks whether a token is a valid register name.
 *
 * @param token Token to classify.
 * @return true if the token is a recognized register, false otherwise.
 */
bool is_register(const char *token);

/**
 * @brief Checks whether a token is the .struct directive.
 *
 * @param token Token to classify.
 * @return true if the token matches the struct directive, false otherwise.
 */
bool is_struct(const char *token);

/**
 * @brief Checks whether a token is the .string directive.
 *
 * @param token Token to classify.
 * @return true if the token matches the string directive, false otherwise.
 */
bool is_string(const char *token);

/**
 * @brief Checks whether a token is the .extern directive.
 *
 * @param token Token to classify.
 * @return true if the token matches the extern directive, false otherwise.
 */
bool is_extern(const char *token);

/**
 * @brief Checks whether a token is the .entry directive.
 *
 * @param token Token to classify.
 * @return true if the token matches the entry directive, false otherwise.
 */
bool is_entry(const char *token);

/**
 * @brief Checks whether a token is the .data directive.
 *
 * @param token Token to classify.
 * @return true if the token matches the data directive, false otherwise.
 */
bool is_data(const char *token);
