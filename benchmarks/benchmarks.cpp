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

constexpr int simple = 0, medium = 1, big = 2;

static void BM_Parse(benchmark::State& state)
{
    const long selection = state.range(0);
    const std::string filename = (selection == simple) ? "simple.ark" : ((selection == medium) ? "medium.ark" : "big.ark");
    const std::string code = readFile(filename);

    long long nodes = 0;
    long long backtracks = 0;

    for (auto _ : state)
    {
        Parser parser(code, false);
        parser.parse();

        nodes += parser.ast().list().size();
        backtracks += parser.backtrack_count;
    }

    state.counters["nodesRate"] = benchmark::Counter(nodes, benchmark::Counter::kIsRate);
    state.counters["nodesAvg"] = benchmark::Counter(nodes, benchmark::Counter::kAvgThreads);
    state.counters["backtracksRate"] = benchmark::Counter(backtracks, benchmark::Counter::kIsRate);
    state.counters["backtracksAvg"] = benchmark::Counter(backtracks, benchmark::Counter::kAvgThreads);
}

BENCHMARK(BM_Parse)->Name("Simple - 39 nodes")->Arg(simple)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Parse)->Name("Medium - 83 nodes")->Arg(medium)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Parse)->Name("Big - 665 nodes")->Arg(big)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();