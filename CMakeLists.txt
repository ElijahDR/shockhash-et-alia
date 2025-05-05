cmake_minimum_required(VERSION 3.10)
project(HashingAlgos)

set(CMAKE_CXX_STANDARD 20)
include_directories(include)

# Global options
option(TESTS "Enable Test Compiling" ON)
option(PROFILE "Enable Profiling" OFF)

if(TESTS)
    enable_testing()
    add_subdirectory(tests)
    message(STATUS "Test Compilation is enabled.")
else()
    message(STATUS "Test Compilation is disabled.")
endif()

# Define source files once
set(SOURCES
    src/main.cpp
    src/common/murmurhash.cpp
    src/common/MurmurHash3.cpp
    src/common/utils.cpp
    src/common/golomb_rice.cpp
    src/common/elias_fano.cpp
    src/common/broadword.cpp
    src/common/ribbon.cpp
    src/common/bucket_ribbon.cpp
    src/algos/recsplit.cpp
    src/algos/sichash.cpp
    src/algos/shockhash.cpp
    src/algos/shockhash-rs.cpp
)
add_compile_definitions(STATS)

# ---------------------------
# Normal Optimized Build
# ---------------------------
add_executable(hashing_algos ${SOURCES})
target_compile_definitions(hashing_algos PRIVATE NOMINMAX)

# Set output directory
set_target_properties(hashing_algos PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Optimization flags
if(NOT PROFILE)
    target_compile_options(hashing_algos PRIVATE -Ofast -march=native)
endif()

# ---------------------------
# Debug Build (with DEBUG + STATS)
# ---------------------------
add_executable(hashing_algos_debug ${SOURCES})
target_compile_definitions(hashing_algos_debug PRIVATE DEBUG STATS NOMINMAX)

# Output to same bin folder but different name
set_target_properties(hashing_algos_debug PROPERTIES
    OUTPUT_NAME "hashing_algos"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/debug
)

# Always compile debug build without optimization
target_compile_options(hashing_algos_debug PRIVATE -g)

# ---------------------------
# Optional: Profiling Support
# ---------------------------
if(PROFILE)
    message(STATUS "Profiling is enabled for both builds.")
    target_compile_options(hashing_algos PRIVATE -g -pg)
    target_compile_options(hashing_algos_debug PRIVATE -g -pg) # already has -g
    target_link_libraries(hashing_algos PRIVATE -pg)
    target_link_libraries(hashing_algos_debug PRIVATE -pg)
endif()

# ---------------------------
# Output Directories for Libraries (if needed)
# ---------------------------
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)