#include <benchmark/benchmark.h>
#include "bench_fixture.hpp"

// bench_memory.cpp reports the memory footprint of each container at various sizes
// and densities. The timed loop is trivial and the interesting output is smuggled
// through the benchmark counters (bytes_hibitset, bytes_dynset, bytes_stdset, set_bits),
// so ignore the time columns.

namespace {

std::size_t g_stdset_live_bytes = 0;

template<typename T>
struct CountingAllocator {
    using value_type = T;

    CountingAllocator() = default;
    template<typename W>
    CountingAllocator(const CountingAllocator<W>&) noexcept {}

    T* allocate(std::size_t n) {
        g_stdset_live_bytes += n * sizeof(T);
        return std::allocator<T>{}.allocate(n);
    }
    void deallocate(T* p, std::size_t n) noexcept {
        g_stdset_live_bytes -= n * sizeof(T);
        std::allocator<T>{}.deallocate(p, n);
    }
    template<typename W>
    bool operator==(const CountingAllocator<W>&) const noexcept { return true; }
};

template<std::size_t U>
void BM_memory(benchmark::State& state) {
    Fixture<U> f(state.range(0) / 1000.0);

    // std::set bytes, measured: the allocator is rebound to the tree's node
    // type, so this captures the true per-node footprint, not just the keys.
    g_stdset_live_bytes = 0;
    std::size_t bytes_stdset;
    {
        std::set<std::size_t, std::less<std::size_t>, CountingAllocator<std::size_t>>
            counted(f.stdset.begin(), f.stdset.end());
        bytes_stdset = g_stdset_live_bytes + sizeof(counted);
    }

    std::size_t bytes_hibitset = sizeof(hibitset::bitset<U>);
    std::size_t bytes_dynset = f.dynset.num_blocks() * sizeof(boost::dynamic_bitset<>::block_type)
                             + sizeof(f.dynset);

    for (auto _ : state) {
        benchmark::DoNotOptimize(bytes_hibitset);
    }

    state.counters["bytes_hibitset"] = static_cast<double>(bytes_hibitset);
    state.counters["bytes_dynset"] = static_cast<double>(bytes_dynset);
    state.counters["bytes_stdset"] = static_cast<double>(bytes_stdset);
    state.counters["set_bits"] = static_cast<double>(f.hibitset->count());
}

// memory report, U = 1 << 12, 1 << 18, 1 << 24
// density = 0%, 0.1%, 1%, 5%, 10%, then the usual 25/50/75/100% grid -- the
// std::set-vs-hibitset crossover lives well below 25%, so the low end is finer.
BENCHMARK_TEMPLATE(BM_memory, 1 << 12)
    ->Arg(0)->Arg(1)->Arg(10)->Arg(50)->Arg(100)->Arg(250)->Arg(500)->Arg(750)->Arg(1000);
BENCHMARK_TEMPLATE(BM_memory, 1 << 18)
    ->Arg(0)->Arg(1)->Arg(10)->Arg(50)->Arg(100)->Arg(250)->Arg(500)->Arg(750)->Arg(1000);
BENCHMARK_TEMPLATE(BM_memory, 1 << 24)
    ->Arg(0)->Arg(1)->Arg(10)->Arg(50)->Arg(100)->Arg(250)->Arg(500)->Arg(750)->Arg(1000);

}
