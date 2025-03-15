cmake_minimum_required(VERSION 3.10)
project(HashingAlgos)

set(CMAKE_CXX_STANDARD 23)

# Add the include directory
include_directories(include)

option(DEBUG "Enable debug logging" OFF)

# Pass the DEBUG option to the compiler as a preprocessor definition
if(DEBUG)
    add_compile_definitions(DEBUG)
    message(STATUS "Debugging is enabled.")
else()
    message(STATUS "Debugging is disabled.")
endif()

# List all source files
set(SOURCES
    src/main.cpp
    src/common/murmurhash.cpp
    src/common/utils.cpp
    src/common/golomb_rice.cpp
    src/algos/recsplit.cpp
)

# Create the executable
add_executable(hashing_algos ${SOURCES})

set_target_properties(hashing_algos PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)