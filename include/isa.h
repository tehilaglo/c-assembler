/**
 * @file isa.h
 * @brief Instruction-set and encoding type definitions for the assembler.
 *
 * @author Tehila Cahnaman
 */

#pragma once

#include <stdint.h>

/**
 * @def MAX_INSTR_LEN
 * @brief Maximum instruction mnemonic length.
 */
#define MAX_INSTR_LEN 3

/**
 * @def REGISTERS_NUM
 * @brief Number of supported CPU registers.
 */
#define REGISTERS_NUM 8

/**
 * @def INSTR_SET
 * @brief Number of supported instructions in the ISA.
 */
#define INSTR_SET 16

/**
 * @def B32_MAX_VALUE
 * @brief Number of representable symbols in the custom base-32 alphabet.
 */
#define B32_MAX_VALUE 32

/**
 * @def ABSOLUTE
 * @brief Absolute ARE encoding value.
 */
#define ABSOLUTE 0x0

/**
 * @def EXTERNAL
 * @brief External ARE encoding value.
 */
#define EXTERNAL 0x1

/**
 * @def RELOCATABLE
 * @brief Relocatable ARE encoding value.
 */
#define RELOCATABLE 0x2

#define MOV_INSTR "mov"
#define CMP_INSTR "cmp"
#define ADD_INSTR "add"
#define SUB_INSTR "sub"
#define NOT_INSTR "not"
#define CLR_INSTR "clr"
#define LEA_INSTR "lea"
#define INC_INSTR "inc"
#define DEC_INSTR "dec"
#define JMP_INSTR "jmp"
#define BNE_INSTR "bne"
#define GET_INSTR "get"
#define PRN_INSTR "prn"
#define JSR_INSTR "jsr"
#define RTS_INSTR "rts"
#define HLT_INSTR "hlt"

#define STR_DIRECTIVE ".string"
#define DATA_DIRECTIVE ".data"
#define STRUCT_DIRECTIVE ".struct"
#define ENTRY_DIRECTIVE ".entry"
#define EXTERN_DIRECTIVE ".extern"

/**
 * @struct InstructionDef
 * @brief Describes one instruction mnemonic and its opcode.
 */
typedef struct
{
    char instr[MAX_INSTR_LEN + 1]; /**< Instruction mnemonic string. */
    uint8_t opcode;                /**< Numeric opcode value. */
} InstructionDef;

/**
 * @struct Base32Entry
 * @brief Maps a numeric value to its custom base-32 symbol.
 */
typedef struct
{
    uint8_t value; /**< Decimal value in the range [0, 31]. */
    char symbol;   /**< Custom base-32 character representation. */
} Base32Entry;

/**
 * @struct InstructionWord
 * @brief Encoded machine word for an instruction header.
 */
typedef struct
{
    unsigned short are : 2;           /**< ARE bits. */
    unsigned short dst_addr_mode : 2; /**< Destination addressing mode. */
    unsigned short src_addr_mode : 2; /**< Source addressing mode. */
    unsigned short opcode : 4;        /**< Instruction opcode. */
} InstructionWord;

/**
 * @struct DataWord
 * @brief Encoded machine word for data storage.
 */
typedef struct
{
    unsigned short low : 5;  /**< Lower 5 bits of the encoded data value. */
    unsigned short high : 5; /**< Upper 5 bits of the encoded data value. */
} DataWord;

/**
 * @struct EncodedInstruction
 * @brief Represents one encoded instruction entry in the code image.
 *
 * A code entry may hold a resolved machine word, a referenced symbol name,
 * or both, depending on the encoding stage.
 */
typedef struct
{
    char *symbol_name;             /**< Referenced symbol name, if unresolved/symbolic. */
    InstructionWord *machine_code; /**< Encoded instruction word. */
} EncodedInstruction;

/**
 * @brief Allocates and initializes an instruction machine word.
 *
 * @param are ARE field value.
 * @param dest_op Destination addressing mode.
 * @param src_op Source addressing mode.
 * @param op_code Opcode value.
 * @return Newly allocated instruction word.
 */
InstructionWord *new_instr_word(char are, char dest_op, char src_op, char op_code);

/**
 * @brief Allocates and initializes an encoded instruction entry.
 *
 * @param symbol_name Optional symbol name referenced by the instruction.
 * @param instr_word Optional machine-word payload for the instruction.
 * @return Newly allocated encoded instruction entry.
 */
EncodedInstruction *new_instruction(const char *symbol_name, InstructionWord *instr_word);

/**
 * @brief Allocates and initializes an encoded data word.
 *
 * @param num Numeric data value to encode.
 * @return Newly allocated data word.
 */
DataWord *new_data_word(short num);

/**
 * @brief Returns the instruction mnemonic at a given instruction-table index.
 *
 * @param index Instruction-table index.
 * @return Instruction mnemonic string, or NULL if the index is invalid.
 */
const char *isa_get_instr_name(int index);

/**
 * @brief Returns the register name at a given register index.
 *
 * @param index Register index.
 * @return Register name string, or NULL if the index is invalid.
 */
const char *isa_get_register(int index);

/**
 * @brief Returns the custom base-32 symbol for a numeric value.
 *
 * @param num Numeric value to convert.
 * @return Base-32 symbol character, or -1 if the value is invalid.
 */
char get_b32_symbol(char num);

/**
 * @brief Returns the opcode associated with an instruction mnemonic.
 *
 * @param instr Instruction mnemonic to look up.
 * @return Matching opcode, or (uint8_t)-1 if the mnemonic is not found.
 */
uint8_t isa_get_opcode(const char *instr);
