cmake_minimum_required(VERSION 3.16)
project(LoxInterpreter CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Generate perfect_hash.hpp from keywords.gperf
add_custom_command(
    OUTPUT ${SRC_DIR}/perfect_hash.hpp
    COMMAND gperf --compare-strncmp -t -L C++ ${SRC_DIR}/keywords.gperf > ${SRC_DIR}/perfect_hash.hpp
    COMMAND ${CMAKE_COMMAND} -DINPUT=${SRC_DIR}/perfect_hash.hpp
                             -DOUTPUT=${SRC_DIR}/perfect_hash.hpp
                             -DTARGET=register
                             -P ${CMAKE_SOURCE_DIR}/cmake/strip_string.cmake
    DEPENDS ${SRC_DIR}/keywords.gperf
    COMMENT "Generating perfect_hash.hpp from keywords.gperf"
    VERBATIM
)

set(SOURCES
    ${SRC_DIR}/allocator.cpp
    ${SRC_DIR}/interpreter.cpp
    ${SRC_DIR}/lox.cpp
    ${SRC_DIR}/parser.cpp
    ${SRC_DIR}/scanner.cpp
    ${SRC_DIR}/token.cpp
    ${SRC_DIR}/perfect_hash.hpp # Force dependency on generated file
)

# Create executable target
add_executable(lox ${SOURCES})

# Ensure perfect_hash.hpp is generated before compiling
add_dependencies(lox perfect_hash_gen)

# Alias the custom command so CMake tracks the output
add_custom_target(perfect_hash_gen DEPENDS ${SRC_DIR}/perfect_hash.hpp)
