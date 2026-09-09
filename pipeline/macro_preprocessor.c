/**
 * @file macro_preprocessor.c
 * @brief Implements macro preprocessing for assembler source files.
 *
 * This module detects macro definitions, stores their location in the source
 * file, and expands macro invocations into a generated preprocessed output
 * file. Macro bodies are referenced by file offsets rather than copied into
 * memory during the collection phase.
 *
 * @author Tehila Cahnaman
 */

#include "macro_preprocessor.h"

#include <stdlib.h>
#include <string.h>

#include "file_config.h"
#include "line_parser.h"
#include "logger.h"
#include "macro_table.h"
#include "token_classifier.h"

/**
 * @struct MacroCtx
 * @brief Internal state used during macro preprocessing.
 */
typedef struct
{
    bool macro_detected;               /**< True while inside a macro definition. */
    bool am_fp_is_valid;               /**< False if preprocessing encountered an error. */
    MacroTable macros;                 /**< Collected macro definitions. */
    char name[MAX_MACRO_NAME + 1];     /**< Name of the currently parsed macro. */
    long int start_offset;             /**< File offset where the macro body starts. */
    long int end_offset;               /**< File offset where the macro body ends. */
    FILE *am_fp;                       /**< Output `.am` file. */
    Logger logger;                     /**< File/line context for diagnostics. */
} MacroCtx;

/**
 * @enum MacroCheckResult
 * @brief Status codes for macro declaration parsing and validation.
 */
typedef enum
{
    MACRO_OK = 0,          /**< Macro declaration is valid. */
    MACRO_SYNTAX_ERROR,    /**< Macro declaration exists but is malformed. */
    MACRO_NOT_FOUND        /**< Current line is not a macro declaration. */
} MacroCheckResult;

/**
 * @brief Returns true if @p text starts with @p keyword as a standalone token.
 *
 * The keyword must appear at the beginning of the already-trimmed text and be
 * followed by whitespace, newline, or string termination.
 *
 * @param text Input string to test.
 * @param keyword Keyword to match.
 * @return true if the keyword is the first token, false otherwise.
 */
static bool starts_with_keyword(const char *text, const char *keyword)
{
    if (text == NULL)
        return false;

    const size_t keyword_len = strlen(keyword);

    if (strncmp(text, keyword, keyword_len) != 0)
        return false;

    text += keyword_len;
    return *text == '\0' || *text == '\n' || is_inline_ws(*text);
}

/**
 * @brief Parses the macro name from a declaration line.
 *
 * The parser expects @p cursor to point just after the `macro` keyword.
 *
 * @param cursor Current parsing position after the `macro` keyword.
 * @param macro_name Output buffer for the parsed macro name.
 * @param logger Logger context for diagnostics.
 * @return Parsing result code.
 */
static MacroCheckResult parse_macro_name(char *cursor,
                                         char *macro_name,
                                         const Logger *logger)
{
    int i = 0;

    if (cursor == NULL)
        return MACRO_SYNTAX_ERROR;

    if (*cursor == '\n' || *cursor == '\0')
    {
        log_error("%s%s: line %d: macro declaration is incomplete - missing name\n",
                  logger->file_name, ASM_EXTENSION, logger->line_num);
        return MACRO_SYNTAX_ERROR;
    }

    if (!is_inline_ws(*cursor))
        return MACRO_NOT_FOUND;

    skip_inline_ws(&cursor);

    if (*cursor == '\n' || *cursor == '\0')
    {
        log_error("%s%s: line %d: macro declaration is incomplete - missing name\n",
                  logger->file_name, ASM_EXTENSION, logger->line_num);
        return MACRO_SYNTAX_ERROR;
    }

    for (; *cursor != ' ' && *cursor != '\t' && *cursor != '\n' && *cursor != '\0'; ++cursor, ++i)
    {
        if (*cursor == ';' || *cursor == ':' || *cursor == '.')
        {
            if (i == 0)
            {
                log_error("%s%s: line %d: invalid macro name - must start with a letter and contain only alphanumerics\n",
                          logger->file_name, ASM_EXTENSION, logger->line_num);
            }
            else
            {
                log_error("%s%s: line %d: missing whitespace after macro declaration\n",
                          logger->file_name, ASM_EXTENSION, logger->line_num);
            }
            return MACRO_SYNTAX_ERROR;
        }

        if (i >= MAX_MACRO_NAME)
        {
            log_error("%s%s: line %d: macro name exceeds maximum length\n",
                      logger->file_name, ASM_EXTENSION, logger->line_num);
            return MACRO_SYNTAX_ERROR;
        }

        macro_name[i] = *cursor;
    }

    macro_name[i] = '\0';
    return MACRO_OK;
}

/**
 * @brief Validates that only trailing whitespace remains in the line.
 *
 * @param cursor Current parsing position.
 * @param logger Logger context for diagnostics.
 * @return true if the rest of the line contains only valid trailing whitespace.
 */
static bool macro_has_only_trailing_whitespace(const char *cursor, const Logger *logger)
{
    if (cursor == NULL)
        return false;

    for (; *cursor != '\0'; ++cursor)
    {
        if (*cursor != ' ' && *cursor != '\t' && *cursor != '\n')
        {
            log_error("%s%s: line %d: missing whitespace after macro declaration\n",
                      logger->file_name, ASM_EXTENSION, logger->line_num);
            return false;
        }
    }

    return true;
}

/**
 * @brief Checks whether a macro name conflicts with reserved assembler words.
 *
 * @param macro_name Candidate macro name.
 * @param logger Logger context for diagnostics.
 * @return true if the name is reserved, false otherwise.
 */
static bool is_macro_name_reserved(const char *macro_name, const Logger *logger)
{
    if (is_instruction(macro_name))
    {
        log_error("%s%s: line %d: macro name '%s' conflicts with an instruction mnemonic\n",
                  logger->file_name, ASM_EXTENSION, logger->line_num, macro_name);
        return true;
    }

    if (is_register(macro_name))
    {
        log_error("%s%s: line %d: invalid macro name - '%s' is reserved for registers\n",
                  logger->file_name, ASM_EXTENSION, logger->line_num, macro_name);
        return true;
    }

    if (word_is_directive(macro_name))
    {
        log_error("%s%s: line %d: macro name '%s' conflicts with a directive keyword\n",
                  logger->file_name, ASM_EXTENSION, logger->line_num, macro_name);
        return true;
    }

    return false;
}

/**
 * @brief Validates a complete macro declaration line.
 *
 * @param macros Existing macro table used to reject duplicates.
 * @param cursor Current parsing position after the `macro` keyword.
 * @param macro_name Output buffer for the parsed macro name.
 * @param logger Logger context for diagnostics.
 * @return Validation result code.
 */
static MacroCheckResult validate_macro_definition(const MacroTable *macros,
                                                  char *cursor,
                                                  char *macro_name,
                                                  const Logger *logger)
{
    if (macros == NULL || cursor == NULL)
        return MACRO_SYNTAX_ERROR;

    const MacroCheckResult result = parse_macro_name(cursor, macro_name, logger);

    if (result != MACRO_OK)
        return result;

    skip_inline_ws(&cursor);

    while (*cursor != ' ' && *cursor != '\t' && *cursor != '\n' && *cursor != '\0')
        ++cursor;

    if (!macro_has_only_trailing_whitespace(cursor, logger))
        return MACRO_SYNTAX_ERROR;

    if (is_macro_name_reserved(macro_name, logger))
        return MACRO_SYNTAX_ERROR;

    if (macro_exists(macros, macro_name))
    {
        log_error("%s%s: line %d: macro '%s' redefined - multiple declarations not allowed\n",
                  logger->file_name, ASM_EXTENSION, logger->line_num, macro_name);
        return MACRO_SYNTAX_ERROR;
    }
    return MACRO_OK;
}

/**
 * @brief Returns true if the line begins with the `macro` keyword.
 *
 * Leading spaces and tabs are ignored.
 *
 * @param buf Input line.
 * @return true if the line starts with a macro declaration.
 */
static bool is_macro(char *buf)
{
    skip_inline_ws(&buf);

    return starts_with_keyword(buf, "macro");
}

/**
 * @brief Returns true if the line begins with the `endmacro` keyword only.
 *
 * Leading spaces and tabs are ignored. Any non-whitespace content after the
 * keyword causes the check to fail.
 *
 * @param buf Input line.
 * @return true if the line is a valid `endmacro` line.
 */
static bool is_endmacro(char *buf)
{
    skip_inline_ws(&buf);

    if (!starts_with_keyword(buf, "endmacro"))
        return false;

    buf += strlen("endmacro");

    while (*buf != '\0')
    {
        if (*buf != ' ' && *buf != '\t' && *buf != '\n')
            return false;
        ++buf;
    }
    return true;
}

/**
 * @brief Handles the beginning of a macro definition if present on the line.
 *
 * @param fp Input file stream.
 * @param buf Current source line.
 * @param mctx Macro preprocessing context.
 * @return true if the line was handled as a macro-definition line.
 */
static bool parse_macro_definition(FILE *fp, char *buf, MacroCtx *mctx)
{
    if (fp == NULL)
        return false;

    if (!is_macro(buf) || mctx->macro_detected)
        return false;

    /* Move past the "macro" keyword to validate the declaration tail. */
    char *token = strchr(buf, 'o');
    if (token == NULL)
    {
        mctx->am_fp_is_valid = false;
        return true;
    }
    ++token;

    const MacroCheckResult result = validate_macro_definition(&mctx->macros, token, mctx->name, &mctx->logger);

    if (result == MACRO_OK)
    {
        mctx->macro_detected = true;
        mctx->start_offset = ftell(fp);
        mctx->end_offset = mctx->start_offset;
    }
    else if (result == MACRO_SYNTAX_ERROR)
    {
        mctx->am_fp_is_valid = false;
    }
    else
    {
        /* Treat as a regular line when no valid macro declaration was found. */
        fputs(buf, mctx->am_fp);
    }
    return true;
}

/**
 * @brief Finalizes a macro definition when `endmacro` is reached.
 *
 * @param fp Input file stream.
 * @param buf Current source line.
 * @param mctx Macro preprocessing context.
 * @return true if the line was handled while collecting a macro body.
 */
static bool collect_macro_body(FILE *fp, char *buf, MacroCtx *mctx)
{
    if (fp == NULL || buf == NULL || !mctx->macro_detected)
        return false;

    if (is_endmacro(buf))
    {
        int count = 0;
        while (*buf != '\n' && *buf != '\0')
        {
            ++buf;
            ++count;
        }
        /*
         * ftell(fp) points after the current line because fgets() already read it.
         * Subtract the endmacro line length so the stored range covers only the
         * macro body content between "macro <name>" and "endmacro".
         */
        mctx->end_offset = ftell(fp) - (long int)(count + 1);

        Macro *macro_node = new_macro(mctx->name, mctx->start_offset, mctx->end_offset);
        if (mctx->macros.head == NULL)
            mctx->macros.head = macro_node;
        else
            insert_macro(&mctx->macros, macro_node);

        mctx->macro_detected = false;
    }
    return true;
}

/**
 * @brief Expands a macro invocation if the current line matches a macro name.
 *
 * @param fp Input file stream.
 * @param buf Current source line.
 * @param mctx Macro preprocessing context.
 * @return true if a macro invocation was expanded.
 */
static bool expand_macro_invocation(FILE *fp, char *buf, const MacroCtx *mctx)
{
    if (fp == NULL || mctx->am_fp == NULL)
        return false;

    if (mctx->macros.head == NULL)
        return false;

    Macro *macro_node = get_macro(&mctx->macros, buf);

    if (macro_node == NULL)
        return false;

    const long current_offset = ftell(fp);

    if (fseek(fp, macro_node->start_offset, SEEK_SET) != 0)
        fatal_error("failed to seek to macro body for '%s'", macro_node->name);

    const size_t body_len = (size_t) (macro_node->end_offset - macro_node->start_offset);

    char *tmp = malloc(body_len + 1);

    if (tmp == NULL)
        fatal_error("memory allocation failed while expanding macro '%s'", macro_node->name);

    const size_t bytes_read = fread(tmp, 1, body_len, fp);

    if (bytes_read != body_len)
    {
        free(tmp);
        fatal_error("failed to read macro body for '%s'", macro_node->name);
    }

    tmp[body_len] = '\0';
    fwrite(tmp, 1, body_len, mctx->am_fp);

    free(tmp);

    if (fseek(fp, current_offset, SEEK_SET) != 0)
        fatal_error("failed to restore source file position after expanding macro '%s'", macro_node->name);

    return true;
}

/**
 * @brief Preprocesses an assembly file and expands macro invocations.
 *
 * The function reads the input file, detects macro definitions, stores their
 * body offsets, and writes a generated `.am` file containing the expanded
 * source. If preprocessing detects invalid macro syntax, the returned file
 * pointer is NULL.
 *
 * @param fp Input assembly file stream.
 * @param file_name Base file name without extension.
 * @return FILE pointer to the generated preprocessed file, or NULL on failure.
 */
FILE *preprocess(FILE *fp, char *file_name)
{
    char buf[MAX_LINE_LEN + 1];
    char file_name_copy[MAX_FILE_NAME + 4];

    MacroCtx macro_ctx = {
        false,
        true,
        { NULL },
        "",
        0,
        0,
        NULL,
        { file_name, 0 }
    };

    if (fp == NULL || file_name == NULL)
        fatal_error("preprocess failed: input file pointer or file name is NULL");

    snprintf(file_name_copy, sizeof(file_name_copy), "%s%s", file_name, AUTMK_EXTENSION);

    macro_ctx.am_fp = fopen(file_name_copy, "w+");
    if (macro_ctx.am_fp == NULL)
        fatal_error("failed to open file '%s'", file_name_copy);

    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        ++macro_ctx.logger.line_num;

        if (parse_macro_definition(fp, buf, &macro_ctx))
            continue;

        if (collect_macro_body(fp, buf, &macro_ctx))
            continue;

        if (expand_macro_invocation(fp, buf, &macro_ctx))
            continue;

        fputs(buf, macro_ctx.am_fp);
    }

    free_macros(&macro_ctx.macros);

    if (!macro_ctx.am_fp_is_valid)
    {
        fclose(macro_ctx.am_fp);
        return NULL;
    }

    return macro_ctx.am_fp;
}
