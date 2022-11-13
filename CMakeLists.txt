cmake_minimum_required(VERSION 3.24)

project(parser)

include(cmake/sanitizers.cmake)
option(ENABLE_SANITIZERS "Enable ASAN and UBSAN" Off)

add_executable(parser
    src/main.cpp
    src/combinator.cpp
    src/node.cpp
    src/parser.cpp
)

if (ENABLE_SANITIZERS)
    message(STATUS "Enabling address sanitizer and undefined behavior sanitizer")
    add_address_sanitizer()
    add_undefined_sanitizer()
endif()

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)