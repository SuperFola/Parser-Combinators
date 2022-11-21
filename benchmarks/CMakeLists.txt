cmake_minimum_required(VERSION 3.24)

project(bench)

set(BENCHMARK_ENABLE_TESTING Off)
add_subdirectory(../gbench gbench)
add_executable(bench
    benchmarks.cpp
    ../src/baseparser.cpp
    ../src/node.cpp
    ../src/parser.cpp
)
target_link_libraries(bench benchmark::benchmark)

if (MSVC)
    target_compile_options(bench PRIVATE /W4)
else()
    target_compile_options(bench PRIVATE -Wall -Wextra -Wshadow -Wconversion -Wpedantic -Werror)
endif()

set_target_properties(
    bench
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)