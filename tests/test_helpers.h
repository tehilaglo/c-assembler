#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <stdio.h>
#include <stdbool.h>
#include "assembler.h"

/**
 * @brief Creates a temporary file with the specified extension and content.
 *
 * @param path_out Buffer to store the resulting file path.
 * @param path_size Size of the path_out buffer.
 * @param base_name Base file name (without extension).
 * @param extension File extension (e.g. ".as").
 * @param content Content to write into the file.
 * @return true on success, false on error.
 */
bool create_temp_as_file(char *path_out, size_t path_size, const char *base_name, const char *extension, const char *content);

/**
 * @brief Reads the entire contents of a file into a newly allocated string.
 *
 * Caller is responsible for free()ing the returned string.
 *
 * @param filepath Path to the file.
 * @return Null-terminated string containing file contents, or NULL on error.
 */
char *read_file_content(const char *filepath);

/**
 * @brief Checks whether a file exists and is accessible.
 *
 * @param filepath Path to the file.
 * @return true if the file exists, false otherwise.
 */
bool file_exists(const char *filepath);

/**
 * @brief Removes a file if it exists.
 *
 * @param filepath Path to the file.
 */
void remove_file_if_exists(const char *filepath);

/**
 * @brief Removes all potential assembler artifacts for a given base file path (.as, .am, .ob, .ent, .ext).
 *
 * @param base_path Base path without extension.
 */
void cleanup_assembler_artifacts(const char *base_path);

/**
 * @brief Initializes an Assembler context for unit tests.
 *
 * @param asm Assembler pointer to initialize.
 * @param file_name Base file name for logger.
 */
void init_test_assembler(Assembler *asm_ctx, const char *file_name);

/**
 * @brief Cleans up and frees an Assembler context.
 *
 * @param asm Assembler pointer to clean up.
 */
void cleanup_test_assembler(Assembler *asm_ctx);

/**
 * @brief Checks if a file's content matches the expected string.
 *
 * @param filepath Path to the file.
 * @param expected Expected content.
 * @return true if match, false otherwise.
 */
bool check_file_matches_string(const char *filepath, const char *expected);

#endif /* TEST_HELPERS_H */
