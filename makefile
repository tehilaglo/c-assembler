# ==============================
# Compiler and build settings
# ==============================

# C compiler to use
CC := clang

# Name of the final executable
TARGET := assembler

# Directory where object files will be stored
BUILD_DIR := build

# Compiler flags:
# -std=c99      → compile using the C99 standard
# -Wall         → enable most compiler warnings
# -Wextra       → enable additional warnings
# -Wpedantic    → enforce strict ISO C compliance
# -g            → include debug symbols (for gdb / lldb)
# -Iinclude     → add "include" directory to header search path
CFLAGS := -std=c99 -Wall -Wextra -Wpedantic -g -Iinclude


# ==============================
# Project structure
# ==============================

# Directories containing source files
SRC_DIRS := driver encoder io parser pipeline semantics symbols utils

# Collect all .c source files from the listed directories
SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))

# Convert source file paths to object file paths inside BUILD_DIR
# Example: parser/file.c → build/parser/file.o
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

# Dependency files generated automatically by the compiler
# (.d files track header dependencies)
DEPS := $(OBJS:.o=.d)


# ==============================
# Main build targets
# ==============================

# Phony targets do not correspond to actual files
.PHONY: all clean rebuild run

# Default target executed when running `make`
all: $(TARGET)

# Link all object files into the final executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

# Compile each .c file into an object file
# $< = first dependency (source file)
# $@ = target file (object file)
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)       # create directory if it doesn't exist
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Include automatically generated dependency files
# This ensures files are rebuilt when headers change
-include $(DEPS)


# ==============================
# Cleanup and rebuild
# ==============================

# Remove compiled objects and executable
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	rm -rf output_files

# Rebuild project from scratch
rebuild: clean all


# ==============================
# Run helper
# ==============================

# Run the assembler with a specified input file
# Example: make run FILE=examples/valid/as_example_1
run: $(TARGET)
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make run FILE=examples/valid/as_example_1"; \
		exit 1; \
	fi
	./$(TARGET) $(FILE)


# ==============================
# Optional testing helpers
# (match scripts in the scripts/ directory)
# ==============================

.PHONY: test-valid test-pass1 test-pass2 test-prepro show-output

# Run tests on valid assembly files
test-valid: $(TARGET)
	sh scripts/valid_as_files_testing.sh

# Run tests that should trigger pass1 errors
test-pass1: $(TARGET)
	sh scripts/pass1_errors_testing.sh

# Run tests that should trigger pass2 errors
test-pass2: $(TARGET)
	sh scripts/pass2_errors_testing.sh

# Run tests for macro preprocessing errors
test-prepro: $(TARGET)
	sh scripts/prepro_errors_testing.sh

# Show generated output files (.ob, .ent, .ext)
show-output:
	sh scripts/show_output_files.sh