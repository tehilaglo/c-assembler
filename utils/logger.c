/**
 * @file logger.c
 * @brief Logging utilities for assembler diagnostics and fatal errors.
 *
 * This module provides helper functions for reporting:
 * - errors,
 * - warnings,
 * - informational messages,
 * - fatal errors that terminate the program.
 *
 * Colored prefixes are used to improve readability in terminal output.
 *
 * @author Tehila Cahnaman
 */

#include "logger.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Logs an error message to stderr.
 *
 * The message is prefixed with a colored "[Error]" label.
 *
 * @param fmt printf-style format string.
 * @param ... Format arguments.
 */
void log_error(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    fprintf(stderr, RED "[Error] " RESET);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

/**
 * @brief Logs a warning message to stderr.
 *
 * The message is prefixed with a colored "[Warning]" label.
 *
 * @param fmt printf-style format string.
 * @param ... Format arguments.
 */
void log_warning(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    fprintf(stderr, YELLOW "[Warning] " RESET);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

/**
 * @brief Logs an informational message to stdout.
 *
 * The message is prefixed with a colored "[Info]" label.
 *
 * @param fmt printf-style format string.
 * @param ... Format arguments.
 */
void log_info(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    fprintf(stdout, BLUE "[Info] " RESET);
    vfprintf(stdout, fmt, args);
    va_end(args);
}

/**
 * @brief Logs a fatal error message and terminates the program.
 *
 * If errno is set to a non-zero value, the corresponding system error message
 * is appended to the output. The program then exits with EXIT_FAILURE.
 *
 * @param fmt printf-style format string.
 * @param ... Format arguments.
 */
void fatal_error(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    fprintf(stderr, "Fatal Error: ");
    vfprintf(stderr, fmt, args);

    if (errno != 0)
        fprintf(stderr, ": %s", strerror(errno));

    fprintf(stderr, "\n");
    va_end(args);

    exit(EXIT_FAILURE);
}
