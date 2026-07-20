#include <benchmark/benchmark.h>
#include "bench_fixture.hpp"

// bench_algebra.cpp aims to measure the throughput of in-place set algebra (|=, &=, ^=) 
// on two independently filled operands of the same size and density. 
// The operands are precomputed and shuffled outside the timed loop. 
// The operation is applied repeatedly to the same left operand without resetting it.

namespace {

// Two independently filled operands at the same size and density.
template<std::size_t U>
struct AlgebraFixture {
    Fixture<U> a;
    Fixture<U> b;
    explicit AlgebraFixture(double density)
        : a(density), b(density, 1 << 16, 1337) {}
};

template<std::size_t U>
void BM_or_hibitset(benchmark::State& state) {
    AlgebraFixture<U> f(state.range(0) / 1000.0);
    for (auto _ : state) {
        benchmark::DoNotOptimize(*f.a.hibitset |= *f.b.hibitset);
    }
}

template<std::size_t U>
void BM_or_stdset(benchmark::State& state) {
    AlgebraFixture<U> f(state.range(0) / 1000.0);
    for (auto _ : state) {
        std::set<std::size_t> out;
        std::ranges::set_union(f.a.stdset, f.b.stdset, std::inserter(out, out.end()));
        benchmark::DoNotOptimize(out.size());
    }
}

template<std::size_t U>
void BM_or_dynset(benchmark::State& state) {
    AlgebraFixture<U> f(state.range(0) / 1000.0);
    for (auto _ : state) {
        benchmark::DoNotOptimize(f.a.dynset |= f.b.dynset);
    }
}

template<std::size_t U>
void BM_and_hibitset(benchmark::State& state) {
    AlgebraFixture<U> f(state.range(0) / 1000.0);
    for (auto _ : state) {
        benchmark::DoNotOptimize(*f.a.hibitset &= *f.b.hibitset);
    }
}

template<std::size_t U>
void BM_and_stdset(benchmark::State& state) {
    AlgebraFixture<U> f(state.range(0) / 1000.0);
    for (auto _ : state) {
        std::set<std::size_t> out;
        std::ranges::set_intersection(f.a.stdset, f.b.stdset, std::inserter(out, out.end()));
        benchmark::DoNotOptimize(out.size());
    }
}

template<std::size_t U>
void BM_and_dynset(benchmark::State& state) {
    AlgebraFixture<U> f(state.range(0) / 1000.0);
    for (auto _ : state) {
        benchmark::DoNotOptimize(f.a.dynset &= f.b.dynset);
    }
}

template<std::size_t U>
void BM_xor_hibitset(benchmark::State& state) {
    AlgebraFixture<U> f(state.range(0) / 1000.0);
    for (auto _ : state) {
        benchmark::DoNotOptimize(*f.a.hibitset ^= *f.b.hibitset);
    }
}

template<std::size_t U>
void BM_xor_stdset(benchmark::State& state) {
    AlgebraFixture<U> f(state.range(0) / 1000.0);
    for (auto _ : state) {
        std::set<std::size_t> out;
        std::ranges::set_symmetric_difference(f.a.stdset, f.b.stdset, std::inserter(out, out.end()));
        benchmark::DoNotOptimize(out.size());
    }
}

template<std::size_t U>
void BM_xor_dynset(benchmark::State& state) {
    AlgebraFixture<U> f(state.range(0) / 1000.0);
    for (auto _ : state) {
        benchmark::DoNotOptimize(f.a.dynset ^= f.b.dynset);
    }
}

// algebra Benchmark, U = 1 << 12, 1 << 18, 1 << 24, density = 0.05, 0.5, 0.95
BENCHMARK_TEMPLATE(BM_or_hibitset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_or_hibitset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_or_hibitset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_or_stdset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_or_stdset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_or_stdset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_or_dynset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_or_dynset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_or_dynset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);

BENCHMARK_TEMPLATE(BM_and_hibitset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_and_hibitset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_and_hibitset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_and_stdset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_and_stdset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_and_stdset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_and_dynset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_and_dynset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_and_dynset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);

BENCHMARK_TEMPLATE(BM_xor_hibitset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_xor_hibitset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_xor_hibitset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_xor_stdset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_xor_stdset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_xor_stdset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_xor_dynset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_xor_dynset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_xor_dynset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);

}
