/**
 * @file obj_writer.c
 * @brief Writes the assembler object image to an output .ob file.
 *
 * This module is responsible for generating the object file produced by the
 * assembler. The file contains:
 * - a header line with the instruction count (IC) and data count (DC),
 * - the encoded instruction image,
 * - the encoded data image.
 *
 * Values are written using the project's custom base-32 representation.
 *
 * @author Tehila Cahnaman
 */

#include "obj_writer.h"

#include <stdio.h>

#include "assembler.h"
#include "base32_io.h"
#include "file_config.h"
#include "logger.h"

/**
 * @brief Writes a single counter value to the object-file header.
 *
 * Values smaller than the base-32 symbol table size are written as a single
 * symbol, while larger values are written as a two-symbol base-32 address.
 *
 * @param ob_file Open output file stream.
 * @param value Counter value to write.
 */
static void write_header_count(FILE *ob_file, const int value)
{
    if (value >= B32_MAX_VALUE)
        f_put_b32_address(ob_file, value);
    else
        fputc(get_b32_symbol((char)value), ob_file);
}

/**
 * @brief Encodes and writes one instruction word as two base-32 symbols.
 *
 * The instruction word is split into:
 * - the five most-significant bits,
 * - the five least-significant bits.
 *
 * Bit packing follows the assembler's instruction-word layout.
 *
 * @param ob_file Open output file stream.
 * @param instruction Encoded instruction to write.
 */
static void write_encoded_instruction_word(FILE *ob_file,
                                           const EncodedInstruction *instruction)
{
    /*
     * Five MSB:
     * [ opcode(4 bits) | src_addr_mode high bit(1 bit) ]
     */
    unsigned int packed_value = instruction->machine_code->opcode;
    packed_value <<= 1;

    unsigned int temp = instruction->machine_code->src_addr_mode;
    temp >>= 1;
    packed_value |= temp;

    fputc(get_b32_symbol((char)packed_value), ob_file);

    /*
     * Five LSB:
     * [ src_addr_mode low bit(1 bit) | dst_addr_mode(2 bits) | are(2 bits) ]
     */
    packed_value = instruction->machine_code->src_addr_mode & 1u;
    packed_value <<= 4;

    temp = instruction->machine_code->dst_addr_mode;
    temp <<= 2;
    packed_value |= temp;

    packed_value |= instruction->machine_code->are;

    fputc(get_b32_symbol((char)packed_value), ob_file);
}

/**
 * @brief Opens the output object file and writes its header line.
 *
 * The header format is:
 * <tab>IC DC<newline>
 *
 * Both IC and DC are written using the project's base-32 representation.
 *
 * @param asm Assembler context containing output metadata.
 * @return Open file stream for the object file.
 */
static FILE *open_obj_file_and_write_header(const Assembler *asm)
{
    char file_name[MAX_FILE_NAME + sizeof(OB_EXTENSION)];

    snprintf(file_name, sizeof(file_name), "%s%s", asm->logger.file_name, OB_EXTENSION);

    FILE *ob_file = fopen(file_name, "w");

    if (ob_file == NULL)
        fatal_error("failed to open file '%s'", file_name);

    fputc('\t', ob_file);
    write_header_count(ob_file, asm->ic);

    fputc(' ', ob_file);
    write_header_count(ob_file, asm->dc);

    fputc('\n', ob_file);

    return ob_file;
}

/**
 * @brief Writes the instruction and data images to the object file.
 *
 * Instruction words are written first, followed by data words. Each line
 * contains the base address and the encoded word payload.
 *
 * @param ob_file Open output file stream.
 * @param asm Assembler context containing code and data images.
 */
static void write_obj_file_body(FILE *ob_file, const Assembler *asm)
{
    int i;

    if (ob_file == NULL)
        return;

    /* Write the instruction image. */
    for (i = 0; i < asm->ic; ++i)
    {
        f_put_b32_address(ob_file, INIT_ADDRESS + i);
        fputc('\t', ob_file);
        fputc('\t', ob_file);

        write_encoded_instruction_word(ob_file, asm->code[i]);

        fputc('\n', ob_file);
    }

    /* Write the data image after the instruction image. */
    for (i = 0; i < asm->dc; ++i)
    {
        f_put_b32_address(ob_file, INIT_ADDRESS + asm->ic + i);
        fputc('\t', ob_file);
        fputc('\t', ob_file);

        fputc(get_b32_symbol((char)asm->data[i]->high), ob_file);
        fputc(get_b32_symbol((char)asm->data[i]->low), ob_file);

        fputc('\n', ob_file);
    }
}

/**
 * @brief Generates the object (.ob) file for the assembled program.
 *
 * This function opens the output file, writes the header and object image,
 * and then closes the file.
 *
 * @param asm Assembler context containing the encoded program image.
 */
void to_obj_file(const Assembler *asm)
{
    if (asm == NULL)
        return;

    FILE *ob_file = open_obj_file_and_write_header(asm);
    write_obj_file_body(ob_file, asm);
    fclose(ob_file);
}
