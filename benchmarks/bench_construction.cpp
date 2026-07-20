#include <benchmark/benchmark.h>
#include "bench_fixture.hpp"

// bench_construction.cpp aims to measure the construction cost of each container
// including its own heap allocation and zero-fill, reported as ns per element.
// Populate random distinct indices into a fresh empty container.
// The indices are precomputed and shuffled outside the timed loop.

namespace {

template<std::size_t U>
std::vector<std::size_t> make_shuffled_indices(double density) {
    std::mt19937 rng(42);
    const auto target = static_cast<std::size_t>(density * U);
    // Already both a uniform random subset and in random order (Fisher-Yates
    // guarantees full randomness of the processed prefix), so no extra shuffle needed.
    return sample_distinct_indices<U>(target, rng);
}

template<std::size_t U>
void BM_construct_hibitset(benchmark::State& state) {
    const auto idxs = make_shuffled_indices<U>(state.range(0) / 1000.0);
    for (auto _ : state) {
        auto hb = std::make_unique<hibitset::bitset<U>>();
        for (const std::size_t i : idxs) hb->set(i);
        benchmark::DoNotOptimize(*hb);
    }
    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(idxs.size()));
}

template<std::size_t U>
void BM_construct_stdset(benchmark::State& state) {
    const auto idxs = make_shuffled_indices<U>(state.range(0) / 1000.0);
    for (auto _ : state) {
        std::set<std::size_t> s;
        for (const std::size_t i : idxs) s.insert(i);
        benchmark::DoNotOptimize(s);
    }
    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(idxs.size()));
}

template<std::size_t U>
void BM_construct_dynset(benchmark::State& state) {
    const auto idxs = make_shuffled_indices<U>(state.range(0) / 1000.0);
    for (auto _ : state) {
        boost::dynamic_bitset<> db(U);
        for (const std::size_t i : idxs) db.set(i);
        benchmark::DoNotOptimize(db);
    }
    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(idxs.size()));
}

// construction Benchmark, U = 1 << 12, 1 << 18, 1 << 24, density = 0.05, 0.5, 0.95
BENCHMARK_TEMPLATE(BM_construct_hibitset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_construct_hibitset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_construct_hibitset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_construct_stdset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_construct_stdset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_construct_stdset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_construct_dynset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_construct_dynset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_construct_dynset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);

}
