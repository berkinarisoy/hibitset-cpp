#include <benchmark/benchmark.h>
#include "bench_fixture.hpp"

// bench_mutations.cpp measures the throughput of single-bit mutations (set, clear) on a container, 
// reported as ns per operation via SetItemsProcessed. 

namespace {

template<std::size_t U>
void BM_set_hibitset(benchmark::State& state) {
    Fixture<U> f(state.range(0) / 1000.0);
    std::size_t qi = 0;
    for (auto _ : state) {
        const auto q = f.queries[qi++ & (f.queries.size() - 1)];
        benchmark::DoNotOptimize(f.hibitset->set(q));
    }
}

template<std::size_t U>
void BM_set_stdset(benchmark::State& state) {
    Fixture<U> f(state.range(0) / 1000.0);
    std::size_t qi = 0;
    for (auto _ : state) {
        const auto q = f.queries[qi++ & (f.queries.size() - 1)];
        benchmark::DoNotOptimize(f.stdset.insert(q).second);
    }
}

template<std::size_t U>
void BM_set_dynset(benchmark::State& state) {
    Fixture<U> f(state.range(0) / 1000.0);
    std::size_t qi = 0;
    for (auto _ : state) {
        const auto q = f.queries[qi++ & (f.queries.size() - 1)];
        // test_set(pos, val) returns the bit's *old* value, so !old means it was newly set
        benchmark::DoNotOptimize(!f.dynset.test_set(q, true));
    }
}

template<std::size_t U>
void BM_clear_hibitset(benchmark::State& state) {
    Fixture<U> f(state.range(0) / 1000.0);
    std::size_t qi = 0;
    for (auto _ : state) {
        const auto q = f.queries[qi++ & (f.queries.size() - 1)];
        benchmark::DoNotOptimize(f.hibitset->clear(q));
    }
}

template<std::size_t U>
void BM_clear_stdset(benchmark::State& state) {
    Fixture<U> f(state.range(0) / 1000.0);
    std::size_t qi = 0;
    for (auto _ : state) {
        const auto q = f.queries[qi++ & (f.queries.size() - 1)];
        benchmark::DoNotOptimize(f.stdset.erase(q));
    }
}

template<std::size_t U>
void BM_clear_dynset(benchmark::State& state) {
    Fixture<U> f(state.range(0) / 1000.0);
    std::size_t qi = 0;
    for (auto _ : state) {
        const auto q = f.queries[qi++ & (f.queries.size() - 1)];
        // test_set(pos, val) returns the bit's *old* value, so this reports whether it was cleared
        benchmark::DoNotOptimize(f.dynset.test_set(q, false));
    }
}

// set Benchmark, U = 1 << 12, 1 << 18, 1 << 24, density = 0.05, 0.5, 0.95
BENCHMARK_TEMPLATE(BM_set_hibitset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_set_hibitset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_set_hibitset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_set_stdset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_set_stdset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_set_stdset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_set_dynset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_set_dynset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_set_dynset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);

// clear Benchmark, U = 1 << 12, 1 << 18, 1 << 24, density = 0.05, 0.5, 0.95
BENCHMARK_TEMPLATE(BM_clear_hibitset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_clear_hibitset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_clear_hibitset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_clear_stdset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_clear_stdset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_clear_stdset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_clear_dynset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_clear_dynset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_clear_dynset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);

}