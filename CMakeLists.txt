# Require at least CMake version 3.21
cmake_minimum_required(VERSION 3.21)

# Define the project name and language
project(assembler LANGUAGES C)

# ==============================
# C language configuration
# ==============================

# Use the C99 standard
set(CMAKE_C_STANDARD 99)

# Require the specified C standard (do not fallback to older versions)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Disable compiler-specific language extensions (enforces strict C99)
set(CMAKE_C_EXTENSIONS OFF)

# Place the compiled executable in the project root directory
# (similar to the Makefile behavior)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})


# ==============================
# Compiler warning flags
# ==============================

# Add common warning flags depending on the compiler
# GCC / Clang use Unix-style flags
if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic)

    # MSVC (Windows) uses different flags
elseif(MSVC)
    add_compile_options(/W4)
endif()


# ==============================
# Build configuration message
# ==============================

# Print a message when building in Debug mode
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring Debug build")
endif()


# ==============================
# Project source directories
# ==============================

# List of directories that contain source files
set(SRC_DIRS
        driver
        encoder
        io
        parser
        pipeline
        semantics
        symbols
        utils
)

# ==============================
# Collect source files
# ==============================

# Initialize the list that will store all .c source files
set(SOURCES "")

# Iterate over each source directory
foreach(dir IN LISTS SRC_DIRS)

    # Find all .c files inside the directory
    # CONFIGURE_DEPENDS ensures CMake reconfigures if new files are added
    file(GLOB DIR_SOURCES CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.c"
    )

    # Append the found files to the global SOURCES list
    list(APPEND SOURCES ${DIR_SOURCES})

endforeach()


# ==============================
# Create executable target
# ==============================

# Build the executable using all collected source files
add_executable(${PROJECT_NAME} ${SOURCES})


# ==============================
# Include directories
# ==============================

# Add the "include" directory to the header search path
# Equivalent to the Makefile flag: -Iinclude
target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ==============================
# Makefile-like helper targets
# ==============================

# Extend built-in clean target to also remove output_files and script test logs
# (matching `make clean` behavior: rm -rf output_files, rm -f *_output)
file(GLOB OUTPUT_LOG_FILES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*_output"
)

set(CLEAN_EXTRA_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/output_files"
        "${CMAKE_CURRENT_SOURCE_DIR}/valid_as_examples_output"
        "${CMAKE_CURRENT_SOURCE_DIR}/pass1_errors_output"
        "${CMAKE_CURRENT_SOURCE_DIR}/pass2_errors_output"
        "${CMAKE_CURRENT_SOURCE_DIR}/prepro_errors_output"
        ${OUTPUT_LOG_FILES}
)
list(REMOVE_DUPLICATES CLEAN_EXTRA_FILES)

set_directory_properties(PROPERTIES
        ADDITIONAL_CLEAN_FILES "${CLEAN_EXTRA_FILES}"
)

# Build test runner similarly to Makefile test target
set(LIB_SOURCES ${SOURCES})
list(FILTER LIB_SOURCES EXCLUDE REGEX ".*/driver/main\\.c$")

file(GLOB TEST_HELPER_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_helpers.c"
)

file(GLOB TEST_UNIT_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/*.c"
)

file(GLOB TEST_INT_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/*.c"
)

set(TEST_UNIT_LIB_SOURCES ${TEST_UNIT_SOURCES})
list(FILTER TEST_UNIT_LIB_SOURCES EXCLUDE REGEX ".*/unit_tests_main\\.c$")

set(TEST_INT_LIB_SOURCES ${TEST_INT_SOURCES})
list(FILTER TEST_INT_LIB_SOURCES EXCLUDE REGEX ".*/integration_tests_main\\.c$")

add_executable(test_runner
        ${LIB_SOURCES}
        ${TEST_HELPER_SOURCES}
        ${TEST_UNIT_LIB_SOURCES}
        ${TEST_INT_LIB_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_runner_main.c
)

target_include_directories(test_runner
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

# Run automated tests: cmake --build <build_dir> --target test
add_custom_target(test
        DEPENDS test_runner
        COMMAND $<TARGET_FILE:test_runner>
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running test runner"
)

# Show generated output files: cmake --build <build_dir> --target show-output
add_custom_target(show-output
        COMMAND sh "${CMAKE_CURRENT_SOURCE_DIR}/scripts/show_output_files.sh"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Showing generated output files (.ob, .ent, .ext)"
)
