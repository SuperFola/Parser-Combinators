#include <benchmark/benchmark.h>
#include <fstream>
#include <string>

#include "../src/parser.hpp"

std::string readFile(const std::string& filename)
{
    std::ifstream stream(filename);
    std::string code((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    return code;
}

static void Simple(benchmark::State& state)
{
    const std::string code = readFile("simple.ark");
    for (auto _ : state)
    {
        Parser parser(code, false);
        parser.parse();
    }
}

BENCHMARK(Simple)->Name("Simple - 39 nodes")->Unit(benchmark::kMillisecond);

static void Medium(benchmark::State& state)
{
    const std::string code = readFile("medium.ark");
    for (auto _ : state)
    {
        Parser parser(code, false);
        parser.parse();
    }
}

BENCHMARK(Medium)->Name("Medium - 83 nodes")->Unit(benchmark::kMillisecond);

static void Big(benchmark::State& state)
{
    const std::string code = readFile("big.ark");
    for (auto _ : state)
    {
        Parser parser(code, false);
        parser.parse();
    }
}

BENCHMARK(Big)->Name("Big - 665 nodes")->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();