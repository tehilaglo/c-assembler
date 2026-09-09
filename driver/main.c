/**
 * @file main.c
 * @brief Entry point for the assembler application.
 *
 * This module validates command-line input, opens each requested source file,
 * runs the macro preprocessor, and then invokes the assembler compilation flow
 * on the generated preprocessed file.
 *
 * Expected input arguments are source file names without the `.as` suffix.
 *
 * @author Tehila Cahnaman
 */

#include <stdio.h>
#include <string.h>

#include "assembler.h"
#include "file_config.h"
#include "input_validator.h"
#include "macro_preprocessor.h"

/**
 * @brief Runs the assembler on one or more input source files.
 *
 * Each command-line argument is expected to be a source file name without the
 * assembler extension suffix. For every valid input:
 * - the full source file name is built,
 * - the source file is opened and checked,
 * - macro preprocessing is performed,
 * - the assembler compilation process is executed on the preprocessed output.
 *
 * Invalid file names, inaccessible files, and empty files are reported and
 * skipped without stopping the processing of subsequent inputs.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 *
 * @return Returns 0 when program execution completes.
 */
int main(int argc, char **argv)
{
    char file_name_copy[MAX_FILE_NAME + sizeof(ASM_EXTENSION)];

    if (argc < 2)
    {
        printf("Error: no input files provided. Please specify at least one "
               "source file with the '%s' extension (without suffix).\n",
               ASM_EXTENSION);
    }

    for (int i = 1; i < argc; i++)
    {
        char *file_name = argv[i];

        if (!validate_input_filename(file_name))
        {
            printf("Error: Please specify a source file with the '%s' "
                   "extension without the suffix.\n",
                   ASM_EXTENSION);
            continue;
        }

        if (strlen(file_name) > MAX_FILE_NAME)
        {
            printf("Error: file name '%s' exceeds maximum allowed length "
                   "(%d characters)\n",
                   argv[i],
                   MAX_FILE_NAME);
            continue;
        }

        /* Build the actual input file name by appending the assembler suffix. */
        snprintf(file_name_copy, sizeof(file_name_copy), "%s%s",
                 file_name, ASM_EXTENSION);

        FILE *fp = fopen(file_name_copy, "r");
        if (fp == NULL)
        {
            perror("Error");
            continue;
        }

        /*
         * Empty files are skipped. The file is intentionally left open here
         * because the original control flow is preserved exactly.
         */
        if (is_empty_file(fp, file_name))
        {
            continue;
        }

        /* Rewind after the emptiness check so preprocessing starts at the beginning. */
        fseek(fp, 0, SEEK_SET);

        FILE *am_fp = preprocess(fp, file_name);

        if (am_fp != NULL)
        {
            /* Ensure compilation begins reading the preprocessed file from the start. */
            fseek(am_fp, 0, SEEK_SET);
            assembler_compile(am_fp, file_name);
            fclose(am_fp);
        }
        fclose(fp);
    }
    return 0;
}
