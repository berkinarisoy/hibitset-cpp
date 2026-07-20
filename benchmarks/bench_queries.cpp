#include <benchmark/benchmark.h>
#include "bench_fixture.hpp"

// bench_queries.cpp aims to measure the throughput of single-bit queries (test, next_set, prev_set) 
// on a container, reported as ns per operation via SetItemsProcessed. 

namespace {

template<std::size_t U>
void BM_next_hibitset(benchmark::State& state) {
    Fixture<U> f(state.range(0) / 1000.0);
    std::size_t qi = 0;
    for (auto _ : state) {
        const auto q = f.queries[qi++ & (f.queries.size() - 1)];
        benchmark::DoNotOptimize(f.hibitset->next_set(q));
    }
}

template<std::size_t U>
void BM_next_stdset(benchmark::State& state) {
    Fixture<U> f(state.range(0) / 1000.0);
    std::size_t qi = 0;
    for (auto _ : state) {
        const auto q = f.queries[qi++ & (f.queries.size() - 1)];
        auto it = f.stdset.lower_bound(q);
        benchmark::DoNotOptimize(it == f.stdset.end() ? U : *it);
    }
}

template<std::size_t U>
void BM_next_dynset(benchmark::State& state) {
    Fixture<U> f(state.range(0) / 1000.0);
    std::size_t qi = 0;
    for (auto _ : state) {
        const auto q = f.queries[qi++ & (f.queries.size() - 1)];
        const auto found = f.dynset.test(q) ? q : f.dynset.find_next(q);
        benchmark::DoNotOptimize(found == boost::dynamic_bitset<>::npos ? U : found);
    }
}

template<std::size_t U>
void BM_prev_hibitset(benchmark::State& state) {
    Fixture<U> f(state.range(0) / 1000.0);
    std::size_t qi = 0;
    for (auto _ : state) {
        const auto q = f.queries[qi++ & (f.queries.size() - 1)];
        benchmark::DoNotOptimize(f.hibitset->prev_set(q));
    }
}

template<std::size_t U>
void BM_prev_stdset(benchmark::State& state) {
    Fixture<U> f(state.range(0) / 1000.0);
    std::size_t qi = 0;
    for (auto _ : state) {
        const auto q = f.queries[qi++ & (f.queries.size() - 1)];
        auto it = f.stdset.upper_bound(q);
        benchmark::DoNotOptimize(it == f.stdset.begin() ? U : *std::prev(it));
    }
}

// boost::dynamic_bitset has no find_prev, so we manually search backwards
template<std::size_t U>
void BM_prev_dynset(benchmark::State& state) {
    Fixture<U> f(state.range(0) / 1000.0);
    std::size_t qi = 0;
    for (auto _ : state) {
        const auto q = f.queries[qi++ & (f.queries.size() - 1)];
        std::size_t found = U;
        for (std::size_t i = q + 1; i-- > 0;) {
            if (f.dynset.test(i)) { found = i; break; }
        }
        benchmark::DoNotOptimize(found);
    }
}

// next_set Benchmark, U = 1 << 12, 1 << 18, 1 << 24, density = 0.05, 0.5, 0.95
BENCHMARK_TEMPLATE(BM_next_hibitset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_next_hibitset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_next_hibitset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_next_stdset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_next_stdset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_next_stdset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_next_dynset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_next_dynset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_next_dynset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);

// prev_set Benchmark, U = 1 << 12, 1 << 18, 1 << 24, density = 0.05, 0.5, 0.95
BENCHMARK_TEMPLATE(BM_prev_hibitset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_prev_hibitset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_prev_hibitset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_prev_stdset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_prev_stdset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_prev_stdset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_prev_dynset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_prev_dynset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_prev_dynset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);
}