#include <benchmark/benchmark.h>
#include "bench_fixture.hpp"

// bench_iteration.cpp aims to measure the throughput of iterating over all 
// set bits in a container, reported as ns per element via SetItemsProcessed.
// The iteration is applied repeatedly to the same container without resetting it.

namespace {

template<std::size_t U>
void BM_iterate_hibitset(benchmark::State& state) {
    Fixture<U> f(state.range(0) / 1000.0);
    for (auto _ : state) {
        std::size_t sum = 0;
        for (const std::size_t idx : *f.hibitset) sum += idx;
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(f.hibitset->count()));
}

template<std::size_t U>
void BM_iterate_stdset(benchmark::State& state) {
    Fixture<U> f(state.range(0) / 1000.0);
    for (auto _ : state) {
        std::size_t sum = 0;
        for (const std::size_t idx : f.stdset) sum += idx;
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(f.stdset.size()));
}

template<std::size_t U>
void BM_iterate_dynset(benchmark::State& state) {
    Fixture<U> f(state.range(0) / 1000.0);
    for (auto _ : state) {
        std::size_t sum = 0;
        for (auto i = f.dynset.find_first(); i != boost::dynamic_bitset<>::npos; i = f.dynset.find_next(i))
            sum += i;
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(f.dynset.count()));
}

// iteration Benchmark, U = 1 << 12, 1 << 18, 1 << 24, density = 0.05, 0.5, 0.95
BENCHMARK_TEMPLATE(BM_iterate_hibitset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_iterate_hibitset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_iterate_hibitset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_iterate_stdset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_iterate_stdset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_iterate_stdset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_iterate_dynset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_iterate_dynset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_iterate_dynset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);

}
