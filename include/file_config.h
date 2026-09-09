/**
 * @file file_config.h
 * @brief Central file-format and assembler configuration constants.
 *
 * This header defines:
 * - supported file extensions used throughout the assembler pipeline,
 * - the maximum supported input file-name length,
 * - the initial memory address used for code generation.
 *
 * These constants are shared by modules responsible for preprocessing,
 * parsing, encoding, and output-file generation.
 *
 * @author Tehila Cahnaman
 */

#pragma once

/**
 * @def ASM_EXTENSION
 * @brief Extension used for raw assembler source files.
 */
#define ASM_EXTENSION ".as"

/**
 * @def AUTMK_EXTENSION
 * @brief Extension used for preprocessed assembler files after macro expansion.
 */
#define AUTMK_EXTENSION ".am"

/**
 * @def OB_EXTENSION
 * @brief Extension used for generated object output files.
 */
#define OB_EXTENSION ".ob"

/**
 * @def ENT_EXTENSION
 * @brief Extension used for generated entry symbol output files.
 */
#define ENT_EXTENSION ".ent"

/**
 * @def EXT_EXTENSION
 * @brief Extension used for generated external symbol output files.
 */
#define EXT_EXTENSION ".ext"

/**
 * @def MAX_FILE_NAME
 * @brief Maximum supported base file-name length.
 */
#define MAX_FILE_NAME 2048

/**
 * @def INIT_ADDRESS
 * @brief Initial memory address assigned to the first encoded word.
 */
#define INIT_ADDRESS 100
