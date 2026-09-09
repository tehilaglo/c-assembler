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