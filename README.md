# Two-Pass Assembler in C

A modular **two-pass assembler** written in **C (C99)** that translates assembly source files into machine code for a custom 10-bit instruction set architecture (ISA). The assembler handles macro expansion, two-pass compilation, symbol resolution, and output file generation encoded in a custom Base-32 format.

---

## Table of Contents

- [Overview](#overview)
- [Tech Stack & Architecture](#tech-stack--architecture)
  - [Pipeline Flow](#pipeline-flow)
- [Project Structure](#project-structure)
- [Requirements](#requirements)
- [Setup & Build](#setup--build)
  - [Using GNU Make](#using-gnu-make-recommended)
  - [Using CMake](#using-cmake)
- [Usage & Execution](#usage--execution)
  - [Command Line Interface](#command-line-interface)
  - [Running with Make](#running-with-make)
  - [Generated Output Files](#generated-output-files)
- [Testing & Verification](#testing--verification)
  - [Automated C Test Suite](#automated-c-test-suite)
  - [Batch Shell Test Scripts](#batch-shell-test-scripts)
  - [Inspecting Generated Output](#inspecting-generated-output)
- [Instruction Set Architecture (ISA) & Encoding](#instruction-set-architecture-isa--encoding)
  - [Instruction Word Layout](#instruction-word-layout)
  - [Supported Instructions](#supported-instructions)
  - [Addressing Modes](#addressing-modes)
  - [ARE Attribute Field](#are-attribute-field)
- [Build Configuration & Environment Variables](#build-configuration--environment-variables)
- [Future Improvements & TODOs](#future-improvements--todos)
- [License](#license)

---

## Overview

This project implements a complete assembly-to-machine-code pipeline demonstrating fundamental compiler construction and systems programming concepts:
- **Lexical Parsing & Token Classification:** Validating syntax, directives, registers, and labels.
- **Macro Preprocessing:** Expanding user-defined macros before compilation.
- **Two-Pass Compilation:**
  - **Pass 1:** Builds symbol tables, validates address modes, and encodes instructions and data words with unresolved labels.
  - **Pass 2:** Resolves symbolic addresses, validates entry/extern declarations, and produces final machine code.
- **Object File Generation:** Emits object code (`.ob`), entry symbol tables (`.ent`), and external reference tables (`.ext`) using custom Base-32 encoding.

---

## Tech Stack & Architecture

- **Language:** C (ISO C99 standard: `-std=c99`)
- **Build Systems:** GNU Make (`makefile`) and CMake (>= 3.21, `CMakeLists.txt`)
- **Compilers:** Clang / GCC (with strict compiler flags: `-Wall -Wextra -Wpedantic -g`)
- **Dependencies:** None (standard C library only)
- **Target OS:** Unix-like systems (macOS, Linux)

### Pipeline Flow

```text
Source File (.as)
       │
       ▼
[Macro Preprocessor] ───► Preprocessed File (.am)
       │
       ▼
[Pass 1 Assembler]
  ├─ Parse lines & labels
  ├─ Build symbol table
  └─ Encode instruction / data image
       │
       ▼
[Pass 2 Assembler]
  ├─ Resolve label addresses & ARE flags
  ├─ Process .entry / .extern directives
  └─ Validate unresolved references
       │
       ▼
[Output Writers] ───────► .ob  (Object code in Base-32)
                          .ent (Entry symbols & addresses)
                          .ext (External symbols & reference addresses)
```

---

## Project Structure

```text
.
├── CMakeLists.txt         # CMake build configuration
├── makefile               # GNU Makefile with build and test targets
├── include/               # Header files defining interfaces and constants
│   ├── assembler.h
│   ├── assembler_pass1.h
│   ├── assembler_pass2.h
│   ├── base32_io.h
│   ├── data_directive_encoder.h
│   ├── ent_writer.h
│   ├── ext_writer.h
│   ├── file_config.h
│   ├── input_validator.h
│   ├── instr_operands_encoder.h
│   ├── instruction_encoder.h
│   ├── isa.h
│   ├── label_parser.h
│   ├── line_parser.h
│   ├── logger.h
│   ├── macro_preprocessor.h
│   ├── macro_table.h
│   ├── obj_writer.h
│   ├── opcode_rules.h
│   ├── operand_parser.h
│   ├── symbol_table.h
│   └── token_classifier.h
├── driver/                # Application entry point
│   └── main.c
├── pipeline/              # Core compilation workflow
│   ├── assembler.c
│   ├── assembler_pass1.c
│   ├── assembler_pass2.c
│   └── macro_preprocessor.c
├── parser/                # Parsing and lexical analysis
│   ├── label_parser.c
│   ├── line_parser.c
│   ├── operand_parser.c
│   └── token_classifier.c
├── encoder/               # Machine code encoding logic
│   ├── data_directive_encoder.c
│   ├── instr_operands_encoder.c
│   └── instruction_encoder.c
├── semantics/             # ISA definitions and opcode validation rules
│   ├── isa.c
│   └── opcode_rules.c
├── symbols/               # Symbol and macro data structures
│   ├── macro_table.c
│   └── symbol_table.c
├── io/                    # Output file generation (.ob, .ent, .ext) and Base-32 I/O
│   ├── base32_io.c
│   ├── ent_writer.c
│   ├── ext_writer.c
│   └── obj_writer.c
├── utils/                 # Utility helpers and input validation
│   ├── input_validator.c
│   └── logger.c
├── examples/              # Example assembly programs
│   ├── valid/             # Valid assembly test cases (as_example_1.as - as_example_8.as)
│   └── errors/            # Invalid assembly files testing error handling
├── tests/                 # Automated test suite
│   ├── test_runner_main.c
│   ├── test_framework.h
│   ├── test_helpers.h / .c
│   ├── unit/              # Unit tests for components
│   └── integration/       # Integration tests across passes and pipeline
└── scripts/               # Shell scripts for batch and error testing
    ├── valid_as_files_testing.sh
    ├── pass1_errors_testing.sh
    ├── pass2_errors_testing.sh
    ├── prepro_errors_testing.sh
    └── show_output_files.sh
```

---

## Requirements

- **C Compiler:** `clang` or `gcc` with C99 support
- **Build Tool:** GNU `make` or `cmake` (version >= 3.21)
- **Environment:** POSIX-compatible shell (macOS or Linux)

---

## Setup & Build

### Using GNU Make (Recommended)

- **Build the assembler executable (`assembler`):**
  ```bash
  make
  # or: make all
  ```

- **Clean build artifacts and output files:**
  ```bash
  make clean
  ```

- **Rebuild from scratch:**
  ```bash
  make rebuild
  ```

### Using CMake

- **Configure build directory:**
  ```bash
  cmake -B build
  ```

- **Build the assembler executable (`assembler`):**
  ```bash
  cmake --build build
  ```

- **Build the test runner executable (`test_runner`):**
  ```bash
  cmake --build build --target test_runner
  ```

- **Clean build artifacts and generated output files:**
  ```bash
  cmake --build build --target clean
  ```

---

## Usage & Execution

### Command Line Interface

The application entry point is in `driver/main.c`. Run the compiled binary by passing one or more assembly file paths **without** the `.as` extension:

```bash
./assembler <file1> [file2 ...]
```

Example:
```bash
./assembler examples/valid/as_example_1 examples/valid/as_example_2
```

### Running with Make

You can quickly run the assembler using the Make `run` target:

```bash
make run FILE=examples/valid/as_example_1
```

### Generated Output Files

For each input file (e.g., `filename.as`), the assembler generates the following output files in `output_files/` or the target directory:

| Extension | File Type | Description |
| :--- | :--- | :--- |
| `.am` | Preprocessed Source | Expanded source code with all macro definitions replaced by their bodies. |
| `.ob` | Object File | Encoded 10-bit machine instructions and data words formatted in custom Base-32. |
| `.ent` | Entry Symbols Table | Symbol names and their resolved decimal/Base-32 memory addresses (generated if `.entry` directives exist). |
| `.ext` | External References Table | External symbol names and the memory addresses where they are referenced (generated if `.extern` symbols are used). |

---

## Testing & Verification

### Automated C Test Suite

The repository includes a comprehensive unit and integration test suite located in `tests/`.

#### Using GNU Make

- **Run all tests (unit + integration):**
  ```bash
  make test
  ```
- **Run unit tests only:**
  ```bash
  make test-unit
  ```
- **Run integration tests only:**
  ```bash
  make test-integration
  ```

#### Using CMake

- **Run all automated tests via CMake test target:**
  ```bash
  cmake --build build --target test
  ```
- **Run the compiled test runner executable directly:**
  ```bash
  ./build/test_runner
  # or: ./test_runner
  ```

### Batch Shell Test Scripts

Automated batch test scripts are located in `scripts/` and integrated into Make targets:

- **Test valid assembly example files:**
  ```bash
  make test-valid
  # or: sh scripts/valid_as_files_testing.sh
  ```
- **Test macro preprocessor error handling:**
  ```bash
  make test-prepro
  # or: sh scripts/prepro_errors_testing.sh
  ```
- **Test Pass 1 error detection:**
  ```bash
  make test-pass1
  # or: sh scripts/pass1_errors_testing.sh
  ```
- **Test Pass 2 error detection:**
  ```bash
  make test-pass2
  # or: sh scripts/pass2_errors_testing.sh
  ```

### Inspecting Generated Output

To inspect the generated output files (`.ob`, `.ent`, `.ext`) after assembling test cases:

- **Using Make:**
  ```bash
  make show-output
  ```
- **Using CMake:**
  ```bash
  cmake --build build --target show-output
  ```
- **Using Direct Shell Script:**
  ```bash
  sh scripts/show_output_files.sh
  ```

---

## Instruction Set Architecture (ISA) & Encoding

The custom architecture utilizes a **10-bit machine word**:

### Instruction Word Layout

| Bits 9–6 (4 bits) | Bits 5–4 (2 bits) | Bits 3–2 (2 bits) | Bits 1–0 (2 bits) |
| :---: | :---: | :---: | :---: |
| Opcode (0–15) | Source Addressing Mode (0–3) | Destination Addressing Mode (0–3) | ARE Attribute |

### Supported Instructions

| Opcode | Mnemonic | Operands | Description |
| :---: | :---: | :---: | :--- |
| `0` | `mov` | 2 (src, dst) | Move source operand to destination |
| `1` | `cmp` | 2 (src, dst) | Compare source and destination operands |
| `2` | `add` | 2 (src, dst) | Add source to destination |
| `3` | `sub` | 2 (src, dst) | Subtract source from destination |
| `4` | `lea` | 2 (src, dst) | Load effective address of source into destination |
| `5` | `clr` | 1 (dst) | Clear destination operand to zero |
| `6` | `not` | 1 (dst) | Bitwise NOT on destination operand |
| `7` | `inc` | 1 (dst) | Increment destination operand |
| `8` | `dec` | 1 (dst) | Decrement destination operand |
| `9` | `jmp` | 1 (dst) | Jump to target label |
| `10` | `bne` | 1 (dst) | Branch to target label if not equal (zero flag not set) |
| `11` | `get` | 1 (dst) | Read character/input to destination |
| `12` | `prn` | 1 (dst) | Print destination value |
| `13` | `jsr` | 1 (dst) | Jump to subroutine |
| `14` | `rts` | 0 | Return from subroutine |
| `15` | `hlt` | 0 | Halt machine execution |

### Addressing Modes

| Mode Value | Name | Syntax | Description |
| :---: | :--- | :--- | :--- |
| `0` | Immediate | `#value` | Direct constant numeric integer value |
| `1` | Direct | `LABEL` | Address of a label in memory |
| `2` | Struct Indexing | `struct.field` | Member access within a struct (`field` is `1` or `2`) |
| `3` | Direct Register | `r0` – `r7` | Value in one of the 8 CPU registers |

### ARE Attribute Field

| Binary | Character | Meaning | Description |
| :---: | :---: | :--- | :--- |
| `00` | `A` | Absolute | Word is self-contained and does not change when relocated. |
| `01` | `E` | External | Symbol is declared external (`.extern`) and resolved by linker. |
| `10` | `R` | Relocatable | Address will be adjusted based on the program loading base address. |

---

## Build Configuration & Environment Variables

| Variable | Description | Example / Default |
| :--- | :--- | :--- |
| `CC` | C compiler executable override for Make | `make CC=gcc` (Default: `clang`) |
| `CFLAGS` | Base compilation flags for Make | `make CFLAGS="-O2 -Iinclude"` |
| `TEST_CFLAGS` | Compilation flags for test targets | `make TEST_CFLAGS="-O0 -g -Iinclude -Itests"` |
| `FILE` | Target assembly input path when using `make run` | `make run FILE=examples/valid/as_example_1` |

---

## Future Improvements & TODOs

- [ ] Support macro arguments and parameters in the macro preprocessor.
- [ ] Implement conditional assembly directives (e.g., `.ifdef`, `.ifndef`).
- [ ] Add relocation tables and cross-file linking support.
- [ ] Implement debug symbol generation and DWARF-like output.
- [ ] Create a companion disassembler tool to decode `.ob` files back to assembly.

---

## License

Distributed under the [MIT License](LICENSE).
