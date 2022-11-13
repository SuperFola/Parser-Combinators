cmake_minimum_required(VERSION 3.24)

project(parser)

add_executable(parser
    src/main.cpp
    src/combinator.cpp
    src/node.cpp
    src/parser.cpp
)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)