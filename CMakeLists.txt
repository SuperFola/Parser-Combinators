cmake_minimum_required(VERSION 3.20)

project(parser)

include(cmake/sanitizers.cmake)
option(ENABLE_SANITIZERS "Enable ASAN and UBSAN" Off)

add_executable(parser
    src/main.cpp
    src/baseparser.cpp
    src/node.cpp
    src/parser.cpp
)

if (MSVC)
    target_compile_options(parser PRIVATE /W4)
else()
    target_compile_options(parser PRIVATE -Wall -Wextra -Wshadow -Wconversion -Wpedantic -Werror)
endif()

if (ENABLE_SANITIZERS)
    message(STATUS "Enabling address sanitizer and undefined behavior sanitizer")
    add_address_sanitizer()
    add_undefined_sanitizer()
endif()

target_compile_features(parser PRIVATE cxx_std_17)