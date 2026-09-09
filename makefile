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
TEST_CFLAGS := $(CFLAGS) -Itests


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

# Objects needed for linking tests (excluding driver/main.o)
LIB_OBJS := $(filter-out $(BUILD_DIR)/driver/main.o,$(OBJS))

# Dependency files generated automatically by the compiler
# (.d files track header dependencies)
DEPS := $(OBJS:.o=.d)

# Test sources and objects
TEST_HELPER_SRCS := tests/test_helpers.c
TEST_HELPER_OBJS := $(BUILD_DIR)/tests/test_helpers.o

TEST_UNIT_SRCS := $(wildcard tests/unit/*.c)
TEST_UNIT_OBJS := $(patsubst tests/unit/%.c,$(BUILD_DIR)/tests/unit/%.o,$(TEST_UNIT_SRCS))
TEST_UNIT_LIB_OBJS := $(filter-out $(BUILD_DIR)/tests/unit/unit_tests_main.o,$(TEST_UNIT_OBJS))

TEST_INT_SRCS := $(wildcard tests/integration/*.c)
TEST_INT_OBJS := $(patsubst tests/integration/%.c,$(BUILD_DIR)/tests/integration/%.o,$(TEST_INT_SRCS))
TEST_INT_LIB_OBJS := $(filter-out $(BUILD_DIR)/tests/integration/integration_tests_main.o,$(TEST_INT_OBJS))

TEST_RUNNER_MAIN_OBJ := $(BUILD_DIR)/tests/test_runner_main.o

# Test runner executables
TEST_RUNNER := $(BUILD_DIR)/test_runner
TEST_UNIT_RUNNER := $(BUILD_DIR)/test_unit
TEST_INT_RUNNER := $(BUILD_DIR)/test_integration


# ==============================
# Main build targets
# ==============================

# Phony targets do not correspond to actual files
.PHONY: all clean rebuild run test test-unit test-integration

# Default target executed when running `make`
all: $(TARGET)

# Link all object files into the final executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

# Compile test files into object files
$(BUILD_DIR)/tests/%.o: tests/%.c
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -MMD -MP -c $< -o $@

# Compile each .c file into an object file
# $< = first dependency (source file)
# $@ = target file (object file)
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)       # create directory if it doesn't exist
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Test executables
$(TEST_RUNNER): $(LIB_OBJS) $(TEST_HELPER_OBJS) $(TEST_UNIT_LIB_OBJS) $(TEST_INT_LIB_OBJS) $(TEST_RUNNER_MAIN_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $^ -o $@

$(TEST_UNIT_RUNNER): $(LIB_OBJS) $(TEST_HELPER_OBJS) $(TEST_UNIT_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $^ -o $@

$(TEST_INT_RUNNER): $(LIB_OBJS) $(TEST_HELPER_OBJS) $(TEST_INT_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $^ -o $@

# Run automated tests
test: $(TEST_RUNNER)
	./$(TEST_RUNNER)

test-unit: $(TEST_UNIT_RUNNER)
	./$(TEST_UNIT_RUNNER)

test-integration: $(TEST_INT_RUNNER)
	./$(TEST_INT_RUNNER)

# Include automatically generated dependency files
# This ensures files are rebuilt when headers change
-include $(DEPS)
-include $(TEST_HELPER_OBJS:.o=.d)
-include $(TEST_UNIT_OBJS:.o=.d)
-include $(TEST_INT_OBJS:.o=.d)
-include $(TEST_RUNNER_MAIN_OBJ:.o=.d)


# ==============================
# Cleanup and rebuild
# ==============================

# Remove compiled objects and output files
clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TEST_RUNNER) $(TEST_UNIT_RUNNER) $(TEST_INT_RUNNER) output_files
	rm -f *_output

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