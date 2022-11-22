cmake_minimum_required(VERSION 3.24)

project(bench)

set(BENCHMARK_ENABLE_TESTING Off)
add_subdirectory(../gbench gbench)
add_executable(bench
    benchmarks.cpp
    ../src/baseparser.cpp
    ../src/node.cpp
    ../src/parser.cpp
    ../legacy_parser/src/Compiler/AST/Lexer.cpp
    ../legacy_parser/src/Compiler/AST/Node.cpp
    ../legacy_parser/src/Compiler/AST/Parser.cpp
    ../legacy_parser/src/Compiler/AST/makeErrorCtx.cpp
)
target_link_libraries(bench benchmark::benchmark)
target_include_directories(bench PUBLIC ../legacy_parser/include)

set_target_properties(
    bench
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)