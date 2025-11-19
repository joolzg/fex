cmake_minimum_required(VERSION 3.16)

# Project name and version
project(Fex VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic -fPIC")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -fPIC")

# Enable position independent code for shared library
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Include directories
include_directories(include)

# Add subdirectories
add_subdirectory(src)

# Enable testing
enable_testing()
add_subdirectory(tests)

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C standard: ${CMAKE_C_STANDARD}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER_ID}")
message(STATUS "Building LD_PRELOAD compatible shared library")