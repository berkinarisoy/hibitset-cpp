#include <benchmark/benchmark.h>
#include "bench_fixture.hpp"

// bench_mutations.cpp measures the throughput of single-bit mutations (set, clear) on a
// container, reported as ns per operation.

namespace {

constexpr std::size_t kBatch = 128;

template<std::size_t U>
void BM_set_hibitset(benchmark::State& state) {
    std::mt19937 rng(42);
    const auto target = static_cast<std::size_t>(state.range(0) / 1000.0 * U);
    auto hb = std::make_unique<hibitset::bitset<U>>();
    for (const std::size_t i : sample_distinct_indices<U>(target, rng)) hb->set(i);

    std::vector<std::size_t> batch;
    auto refill = [&] {
        batch = sample_matching_indices<U>(kBatch, rng, [&](std::size_t i) { return !hb->test(i); });
    };
    refill();
    std::size_t bi = 0;

    for (auto _ : state) {
        if (bi == kBatch) {
            state.PauseTiming();
            for (std::size_t i : batch) hb->clear(i);
            refill();
            bi = 0;
            state.ResumeTiming();
        }
        benchmark::DoNotOptimize(hb->set(batch[bi++]));
    }
}

template<std::size_t U>
void BM_set_stdset(benchmark::State& state) {
    std::mt19937 rng(42);
    const auto target = static_cast<std::size_t>(state.range(0) / 1000.0 * U);
    std::set<std::size_t> s;
    for (const std::size_t i : sample_distinct_indices<U>(target, rng)) s.insert(i);

    std::vector<std::size_t> batch;
    auto refill = [&] {
        batch = sample_matching_indices<U>(kBatch, rng, [&](std::size_t i) { return !s.contains(i); });
    };
    refill();
    std::size_t bi = 0;

    for (auto _ : state) {
        if (bi == kBatch) {
            state.PauseTiming();
            for (std::size_t i : batch) s.erase(i);
            refill();
            bi = 0;
            state.ResumeTiming();
        }
        benchmark::DoNotOptimize(s.insert(batch[bi++]).second);
    }
}

template<std::size_t U>
void BM_set_dynset(benchmark::State& state) {
    std::mt19937 rng(42);
    const auto target = static_cast<std::size_t>(state.range(0) / 1000.0 * U);
    boost::dynamic_bitset<> db(U);
    for (const std::size_t i : sample_distinct_indices<U>(target, rng)) db.set(i);

    std::vector<std::size_t> batch;
    auto refill = [&] {
        batch = sample_matching_indices<U>(kBatch, rng, [&](std::size_t i) { return !db.test(i); });
    };
    refill();
    std::size_t bi = 0;

    for (auto _ : state) {
        if (bi == kBatch) {
            state.PauseTiming();
            for (std::size_t i : batch) db.reset(i);
            refill();
            bi = 0;
            state.ResumeTiming();
        }
        // test_set(pos, val) returns the bit's *old* value, so !old means it was newly set
        benchmark::DoNotOptimize(!db.test_set(batch[bi++], true));
    }
}

template<std::size_t U>
void BM_clear_hibitset(benchmark::State& state) {
    std::mt19937 rng(42);
    const auto target = static_cast<std::size_t>(state.range(0) / 1000.0 * U);
    auto hb = std::make_unique<hibitset::bitset<U>>();
    for (const std::size_t i : sample_distinct_indices<U>(target, rng)) hb->set(i);

    std::vector<std::size_t> batch;
    auto refill = [&] {
        batch = sample_matching_indices<U>(kBatch, rng, [&](std::size_t i) { return hb->test(i); });
    };
    refill();
    std::size_t bi = 0;

    for (auto _ : state) {
        if (bi == kBatch) {
            state.PauseTiming();
            for (std::size_t i : batch) hb->set(i);
            refill();
            bi = 0;
            state.ResumeTiming();
        }
        benchmark::DoNotOptimize(hb->clear(batch[bi++]));
    }
}

template<std::size_t U>
void BM_clear_stdset(benchmark::State& state) {
    std::mt19937 rng(42);
    const auto target = static_cast<std::size_t>(state.range(0) / 1000.0 * U);
    std::set<std::size_t> s;
    for (const std::size_t i : sample_distinct_indices<U>(target, rng)) s.insert(i);

    std::vector<std::size_t> batch;
    auto refill = [&] {
        batch = sample_matching_indices<U>(kBatch, rng, [&](std::size_t i) { return s.contains(i); });
    };
    refill();
    std::size_t bi = 0;

    for (auto _ : state) {
        if (bi == kBatch) {
            state.PauseTiming();
            for (std::size_t i : batch) s.insert(i);
            refill();
            bi = 0;
            state.ResumeTiming();
        }
        benchmark::DoNotOptimize(s.erase(batch[bi++]));
    }
}

template<std::size_t U>
void BM_clear_dynset(benchmark::State& state) {
    std::mt19937 rng(42);
    const auto target = static_cast<std::size_t>(state.range(0) / 1000.0 * U);
    boost::dynamic_bitset<> db(U);
    for (const std::size_t i : sample_distinct_indices<U>(target, rng)) db.set(i);

    std::vector<std::size_t> batch;
    auto refill = [&] {
        batch = sample_matching_indices<U>(kBatch, rng, [&](std::size_t i) { return db.test(i); });
    };
    refill();
    std::size_t bi = 0;

    for (auto _ : state) {
        if (bi == kBatch) {
            state.PauseTiming();
            for (std::size_t i : batch) db.set(i);
            refill();
            bi = 0;
            state.ResumeTiming();
        }
        // test_set(pos, val) returns the bit's *old* value, so this reports whether it was cleared
        benchmark::DoNotOptimize(db.test_set(batch[bi++], false));
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
