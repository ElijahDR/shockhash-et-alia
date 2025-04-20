cmake_minimum_required(VERSION 3.10)
project(HashingAlgos)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

option(DEBUG "Enable debug logging" OFF)

if(DEBUG)
    add_compile_definitions(DEBUG)
    message(STATUS "Debugging is enabled.")
else()
    message(STATUS "Debugging is disabled.")
endif()

option(TESTS "Enable Test Compiling" ON)

if(TESTS)
    enable_testing()
    add_subdirectory(tests)
    message(STATUS "Test Compilation is enabled.")
else()
    message(STATUS "Test Compilation is disabled.")
endif()

option(PROFILE "Enable Profiling" OFF)

if(PROFILE)
    set(CMAKE_BUILD_TYPE Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    message(STATUS "Profling is enabled.")
else()
    message(STATUS "Profiling is disabled.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -march=native")
    message(STATUS "Compiler optimization flags: -Ofast -march=native")
endif()


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(SOURCES
    src/main.cpp
    src/common/murmurhash.cpp
    src/common/MurmurHash3.cpp
    src/common/utils.cpp
    src/common/golomb_rice.cpp
    src/common/elias_fano.cpp
    src/common/broadword.cpp
    src/common/ribbon.cpp
    src/algos/recsplit.cpp
    src/algos/sichash.cpp
    src/algos/shockhash.cpp
)

add_executable(hashing_algos ${SOURCES})

set_target_properties(hashing_algos PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)