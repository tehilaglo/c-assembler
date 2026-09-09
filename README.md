# Two-Pass Assembler in C

## Overview

This project implements a modular **two-pass assembler** written in **C
(C99)**.\
It translates assembly source files into machine code for a custom
instruction set architecture (ISA).\
The assembler performs macro preprocessing, parsing, symbol resolution,
and machine-code generation.

The project demonstrates fundamental **compiler construction and systems
programming concepts** such as: - multi-pass compilation - lexical
parsing - symbol table management - instruction encoding - macro
preprocessing - object file generation

------------------------------------------------------------------------

## Features

### Core Assembler Pipeline

-   Macro preprocessor for expanding user-defined macros
-   Two-pass compilation:
    -   **Pass 1:** symbol table construction and instruction/data
        encoding
    -   **Pass 2:** symbol resolution and output generation
-   Custom instruction set architecture (16 instructions)
-   Addressing mode validation
-   Robust error handling and diagnostics

### Output Files

The assembler generates:

  File     Description
  -------- ---------------------------------------------
  `.ob`    Object file containing encoded machine code
  `.ent`   Entry symbols file
  `.ext`   External symbol references

### Additional Capabilities

-   Custom **Base-32 encoding** for machine words
-   Modular architecture with clearly separated components
-   Automated test scripts
-   Strict compiler flags for safer builds

------------------------------------------------------------------------

## Architecture

The assembler follows a classic **compiler-style pipeline**:

Source File (.as) │ ▼ Macro Preprocessor │ ▼ PASS 1 • Parse lines •
Detect labels • Build symbol table • Encode instructions and data │ ▼
PASS 2 • Resolve symbol references • Process `.entry` directives •
Generate output files │ ▼ Object Files (.ob .ent .ext)

------------------------------------------------------------------------

## Project Structure

    .
    ├── driver          # Program entry point
    │   └── main.c
    │
    ├── pipeline        # Assembler workflow
    │   ├── assembler.c
    │   ├── assembler_pass1.c
    │   ├── assembler_pass2.c
    │   └── macro_preprocessor.c
    │
    ├── parser          # Parsing utilities
    │   ├── label_parser.c
    │   ├── line_parser.c
    │   ├── operand_parser.c
    │   └── token_classifier.c
    │
    ├── encoder         # Instruction and data encoding
    │   ├── instruction_encoder.c
    │   ├── instr_operands_encoder.c
    │   └── data_directive_encoder.c
    │
    ├── semantics       # ISA definitions and opcode rules
    │   ├── isa.c
    │   └── opcode_rules.c
    │
    ├── symbols         # Symbol and macro tables
    │   ├── symbol_table.c
    │   └── macro_table.c
    │
    ├── io              # Output file generation
    │   ├── obj_writer.c
    │   ├── ent_writer.c
    │   └── ext_writer.c
    │
    ├── utils           # Logging and validation helpers
    │   ├── logger.c
    │   └── input_validator.c
    │
    ├── include         # Header files
    ├── examples        # Example assembly programs
    ├── scripts         # Testing scripts
    ├── Makefile
    └── README.md

------------------------------------------------------------------------

## Build Instructions

### Requirements

-   Clang or GCC
-   Make
-   Unix-like environment (Linux / macOS)

### Build

    make

### Clean

    make clean

### Rebuild

    make rebuild

------------------------------------------------------------------------

## Running the Assembler

Run the assembler on a file **without the `.as` extension**.

Example:

    make run FILE=examples/valid/as_example_1

or

    ./assembler examples/valid/as_example_1

------------------------------------------------------------------------

## Example

### Input (`example.as`)

    MAIN:   mov r3, LENGTH
            add r2, r3
            prn r3
            hlt

    LENGTH: .data 6

### Output (`example.ob`)

        04 01
    !a      @b
    !b      #c
    !c      $d
    !d      %e

Machine words are encoded using a **custom base‑32 alphabet**.

------------------------------------------------------------------------

## Instruction Set

Supported instructions:

    mov cmp add sub
    not clr lea inc
    dec jmp bne get
    prn jsr rts hlt

Each instruction is encoded into a **10‑bit machine word**:

  Field                    Bits
  ------------------------ ------
  Opcode                   4
  Source Addressing        2
  Destination Addressing   2
  ARE                      2

------------------------------------------------------------------------

## Testing

Run automated tests:

### Valid programs

    make test-valid

### Pass‑1 errors

    make test-pass1

### Pass‑2 errors

    make test-pass2

### Macro preprocessing errors

    make test-prepro

------------------------------------------------------------------------

## Future Improvements

Potential extensions:

-   Macro arguments
-   Conditional assembly
-   Relocation tables
-   Linking support
-   Debug symbol output
-   Disassembler tool

------------------------------------------------------------------------

## Author

**Tehila Cahnaman**

Computer Science Graduate\
Systems Programming • Low-Level Software • Cybersecurity
