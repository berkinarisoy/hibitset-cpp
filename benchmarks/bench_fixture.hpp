#include <hibitset/hibitset.hpp>
#include <boost/dynamic_bitset.hpp>
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>
#include <numeric>
#include <random>
#include <set>
#include <utility>
#include <vector>

template<std::size_t U>
std::vector<std::size_t> sample_distinct_indices(std::size_t target, std::mt19937& rng) {
    std::vector<std::size_t> pool(U);
    std::iota(pool.begin(), pool.end(), 0);
    for (std::size_t i = 0; i < target; ++i) {
        std::uniform_int_distribution<std::size_t> pick(i, U - 1);
        std::swap(pool[i], pool[pick(rng)]);
    }
    pool.resize(target);
    return pool;
}

template<std::size_t U, typename Pred>
std::vector<std::size_t> sample_matching_indices(std::size_t count, std::mt19937& rng, Pred pred) {
    std::uniform_int_distribution<std::size_t> d(0, U - 1);
    std::vector<std::size_t> result;
    result.reserve(count);
    while (result.size() < count) {
        std::size_t i = d(rng);
        if (pred(i)) result.push_back(i);
    }
    return result;
}

// Fixture for benchmarking three different bitset implementations:
// 1) hibitset::bitset
// 2) std::set
// 3) boost::dynamic_bitset.
template<std::size_t U>
struct Fixture {
    std::unique_ptr<hibitset::bitset<U>> hibitset = std::make_unique<hibitset::bitset<U>>();
    std::set<std::size_t> stdset;
    boost::dynamic_bitset<> dynset{U};
    std::vector<std::size_t> queries;

    explicit Fixture(double density, std::size_t n_queries = 1 << 16, std::uint32_t seed = 42) {
        std::mt19937 rng(seed);
        const auto target = static_cast<std::size_t>(density * U);
        for (const std::size_t i : sample_distinct_indices<U>(target, rng)) {
            hibitset->set(i);
            stdset.insert(i);
            dynset.set(i);
        }

        std::uniform_int_distribution<std::size_t> d(0, U - 1);
        queries.reserve(n_queries);
        for (std::size_t i = 0; i < n_queries; ++i) queries.push_back(d(rng));
    }
};