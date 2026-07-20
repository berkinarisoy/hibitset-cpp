#include <benchmark/benchmark.h>
#include "bench_fixture.hpp"

// bench_workload.cpp aims to measure the throughput of a realistic workload of mixed single-bit 
// mutations and queries on a container, reported as ns per operation via SetItemsProcessed. 

namespace {

enum class OpKind : std::uint8_t { Set, Clear, Next, Prev };
struct Op { OpKind kind; std::size_t idx; };

template<std::size_t U>
std::vector<Op> make_ops(std::size_t n_ops = 1 << 14) {
    std::mt19937 rng(1234);
    std::uniform_int_distribution<std::size_t> anywhere(0, U - 1);
    std::uniform_int_distribution<int> pct(0, 99);
    const auto width = std::max<std::ptrdiff_t>(4, static_cast<std::ptrdiff_t>(U) / 128);
    std::uniform_int_distribution<std::ptrdiff_t> offset(-width, width);

    std::vector<Op> ops;
    ops.reserve(n_ops);
    std::size_t hot = U / 2;
    for (std::size_t i = 0; i < n_ops; ++i) {
        if (pct(rng) < 2) hot = anywhere(rng);
        const auto raw = static_cast<std::ptrdiff_t>(hot) + offset(rng);
        const auto idx = static_cast<std::size_t>(
            std::clamp<std::ptrdiff_t>(raw, 0, static_cast<std::ptrdiff_t>(U) - 1));
        const int k = pct(rng);
        const OpKind kind = k < 40 ? OpKind::Set
                          : k < 80 ? OpKind::Clear
                          : k < 90 ? OpKind::Next
                          : OpKind::Prev;
        ops.push_back({kind, idx});
        hot = idx;
    }
    return ops;
}

template<std::size_t U>
void BM_workload_hibitset(benchmark::State& state) {
    Fixture<U> f(state.range(0) / 1000.0);
    const auto ops = make_ops<U>();
    for (auto _ : state) {
        for (const Op& op : ops) {
            switch (op.kind) {
                case OpKind::Set:   benchmark::DoNotOptimize(f.hibitset->set(op.idx)); break;
                case OpKind::Clear: benchmark::DoNotOptimize(f.hibitset->clear(op.idx)); break;
                case OpKind::Next:  benchmark::DoNotOptimize(f.hibitset->next_set(op.idx)); break;
                case OpKind::Prev:  benchmark::DoNotOptimize(f.hibitset->prev_set(op.idx)); break;
            }
        }
    }
    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(ops.size()));
}

template<std::size_t U>
void BM_workload_stdset(benchmark::State& state) {
    Fixture<U> f(state.range(0) / 1000.0);
    const auto ops = make_ops<U>();
    for (auto _ : state) {
        for (const Op& op : ops) {
            switch (op.kind) {
                case OpKind::Set:
                    benchmark::DoNotOptimize(f.stdset.insert(op.idx).second);
                    break;
                case OpKind::Clear:
                    benchmark::DoNotOptimize(f.stdset.erase(op.idx));
                    break;
                case OpKind::Next: {
                    auto it = f.stdset.lower_bound(op.idx);
                    benchmark::DoNotOptimize(it == f.stdset.end() ? U : *it);
                    break;
                }
                case OpKind::Prev: {
                    auto it = f.stdset.upper_bound(op.idx);
                    benchmark::DoNotOptimize(it == f.stdset.begin() ? U : *std::prev(it));
                    break;
                }
            }
        }
    }
    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(ops.size()));
}

// workload Benchmark, U = 1 << 12, 1 << 18, 1 << 24, density = 0.05, 0.5, 0.95
BENCHMARK_TEMPLATE(BM_workload_hibitset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_workload_hibitset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_workload_hibitset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_workload_stdset, 1 << 12)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_workload_stdset, 1 << 18)->Arg(50)->Arg(500)->Arg(950);
BENCHMARK_TEMPLATE(BM_workload_stdset, 1 << 24)->Arg(50)->Arg(500)->Arg(950);

}
