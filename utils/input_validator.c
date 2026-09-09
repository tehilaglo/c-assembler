/**
 * @file input_validator.c
 * @brief Utility functions for validating assembler input files.
 *
 * This module provides validation helpers used before the assembler
 * pipeline begins execution. It verifies that:
 *  - the input filename is valid and does not contain an extension
 *  - the input file is not empty
 *  - input lines do not exceed the allowed maximum length
 *
 * The functions here are used early in the assembler workflow
 * to prevent unnecessary processing of invalid or malformed input files.
 *
 * @author Tehila Cahnaman
 */

#include "input_validator.h"
#include "file_config.h"
#include "logger.h"
#include "macro_table.h"
#include <string.h>

/**
 * @brief Validate that the provided input filename has no extension.
 *
 * The assembler expects filenames **without the `.as` suffix**.
 * This function ensures the filename does not contain a '.' character
 * after the final directory separator.
 *
 * Example:
 *   Valid:   program
 *   Invalid: program.as
 *
 * @param file_name Input filename (without extension expected).
 *
 * @return true  if filename is valid
 * @return false if filename contains an extension
 */
bool validate_input_filename(const char *file_name)
{
    if (!file_name || *file_name == '\0')
        return false;

    /* Start from the end of the string */
    const char *ptr = file_name + strlen(file_name);

    /* Move backwards until directory separator or start of string */
    while (ptr > file_name && *ptr != '/' && *ptr != '\\')
        --ptr;

    /* Scan forward and ensure no extension exists */
    for (; *ptr != '\0'; ptr++)
    {
        if (*ptr == '.')
            return false;
    }

    return true;
}

/**
 * @brief Check whether the input file is empty or contains only whitespace.
 *
 * The function scans the file line-by-line and performs two checks:
 *
 * 1. Ensures each line does not exceed the maximum allowed length.
 * 2. Determines whether the file contains any non-whitespace characters.
 *
 * If the file only contains whitespace or blank lines, a warning is issued.
 *
 * @param fp        Open file pointer to the source file.
 * @param file_name Base file name (without extension).
 *
 * @return true  if the file is empty or contains only whitespace
 * @return false if the file contains meaningful content
 */
bool is_empty_file(FILE *fp, const char *file_name)
{
    if (!fp || !file_name)
        return true;

    int non_empty_lines = 0;
    int line_num = 0;

    char buf[MAX_LINE_LEN + 5];

    /* Read file line by line */
    while (fgets(buf, sizeof(buf), fp))
    {
        ++line_num;

        /* Validate line length (80 chars + newline) */
        if (strlen(buf) > MAX_LINE_LEN + 1)
        {
            log_error(
                "%s%s: line %d: line exceeds %d characters (max allowed)\n",
                      file_name,
                      ASM_EXTENSION,
                      line_num,
                      MAX_LINE_LEN);
            continue;
        }

        /* Detect if line contains any non-whitespace characters */
        for (const char *cursor = buf; *cursor != '\0' && *cursor != '\n'; cursor++)
        {
            if (*cursor != ' ' && *cursor != '\t')
            {
                non_empty_lines++;
                break;
            }
        }
    }

    if (non_empty_lines == 0)
    {
        log_warning("%s%s: file appears to be empty or only contains whitespace\n",
                    file_name, ASM_EXTENSION);
        return true;
    }

    return false;
}
