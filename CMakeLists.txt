cmake_minimum_required(VERSION 3.24)

project(parser)

add_executable(parser main.cpp combinator.cpp node.cpp parser.cpp)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)