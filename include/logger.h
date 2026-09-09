/**
 * @file logger.h
 * @brief Declarations for logging and diagnostic output helpers.
 *
 * @author Tehila Cahnaman
 */

#pragma once

#define RED    "\x1b[31m"
#define YELLOW "\x1b[33m"
#define GREEN  "\x1b[32m"
#define BLUE   "\x1b[34m"
#define RESET  "\x1b[0m"

/**
 * @struct Logger
 * @brief Stores source context for diagnostics.
 */
typedef struct
{
    char *file_name; /**< Current source file name. */
    int line_num;    /**< Current source line number. */
} Logger;

/**
 * @brief Logs an error message to stderr.
 *
 * @param fmt printf-style format string.
 * @param ... Format arguments.
 */
void log_error(const char *fmt, ...);

/**
 * @brief Logs a warning message to stderr.
 *
 * @param fmt printf-style format string.
 * @param ... Format arguments.
 */
void log_warning(const char *fmt, ...);

/**
 * @brief Logs an informational message to stdout.
 *
 * @param fmt printf-style format string.
 * @param ... Format arguments.
 */
void log_info(const char *fmt, ...);

/**
 * @brief Logs a fatal error message and terminates the program.
 *
 * @param fmt printf-style format string.
 * @param ... Format arguments.
 */
void fatal_error(const char *fmt, ...);
