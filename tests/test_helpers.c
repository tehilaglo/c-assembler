#include "test_helpers.h"
#include <stdlib.h>
#include <string.h>
#include "file_config.h"

bool create_temp_as_file(char *path_out, size_t path_size, const char *base_name, const char *extension, const char *content)
{
    if (path_out == NULL || base_name == NULL || extension == NULL || content == NULL)
        return false;

    snprintf(path_out, path_size, "%s%s", base_name, extension);

    FILE *fp = fopen(path_out, "w");
    if (fp == NULL)
        return false;

    fputs(content, fp);
    fclose(fp);
    return true;
}

char *read_file_content(const char *filepath)
{
    if (filepath == NULL)
        return NULL;

    FILE *fp = fopen(filepath, "r");
    if (fp == NULL)
        return NULL;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (size < 0)
    {
        fclose(fp);
        return NULL;
    }

    char *buf = (char *)malloc((size_t)size + 1);
    if (buf == NULL)
    {
        fclose(fp);
        return NULL;
    }

    size_t read_bytes = fread(buf, 1, (size_t)size, fp);
    buf[read_bytes] = '\0';
    fclose(fp);
    return buf;
}

bool file_exists(const char *filepath)
{
    if (filepath == NULL)
        return false;

    FILE *fp = fopen(filepath, "r");
    if (fp != NULL)
    {
        fclose(fp);
        return true;
    }
    return false;
}

void remove_file_if_exists(const char *filepath)
{
    if (filepath != NULL && file_exists(filepath))
    {
        remove(filepath);
    }
}

void cleanup_assembler_artifacts(const char *base_path)
{
    if (base_path == NULL)
        return;

    char buf[MAX_FILE_NAME + 16];

    snprintf(buf, sizeof(buf), "%s%s", base_path, ASM_EXTENSION);
    remove_file_if_exists(buf);

    snprintf(buf, sizeof(buf), "%s%s", base_path, AUTMK_EXTENSION);
    remove_file_if_exists(buf);

    snprintf(buf, sizeof(buf), "%s%s", base_path, OB_EXTENSION);
    remove_file_if_exists(buf);

    snprintf(buf, sizeof(buf), "%s%s", base_path, ENT_EXTENSION);
    remove_file_if_exists(buf);

    snprintf(buf, sizeof(buf), "%s%s", base_path, EXT_EXTENSION);
    remove_file_if_exists(buf);
}

void init_test_assembler(Assembler *asm_ctx, const char *file_name)
{
    if (asm_ctx == NULL)
        return;

    asm_ctx->code = NULL;
    asm_ctx->ic = 0;
    asm_ctx->data = NULL;
    asm_ctx->dc = 0;
    asm_ctx->symbols.head = NULL;
    asm_ctx->logger.file_name = (char *)file_name;
    asm_ctx->logger.line_num = 1;

    assembler_init(asm_ctx);

    if (asm_ctx->code != NULL)
        memset(asm_ctx->code, 0, sizeof(EncodedInstruction *) * MAX_FREE_MEM);
    if (asm_ctx->data != NULL)
        memset(asm_ctx->data, 0, sizeof(DataWord *) * MAX_FREE_MEM);
}

void cleanup_test_assembler(Assembler *asm_ctx)
{
    if (asm_ctx == NULL)
        return;

    free_mem(asm_ctx);
}

bool check_file_matches_string(const char *filepath, const char *expected)
{
    if (filepath == NULL || expected == NULL)
        return false;

    char *actual = read_file_content(filepath);
    if (actual == NULL)
        return false;

    bool match = (strcmp(actual, expected) == 0);
    free(actual);
    return match;
}
