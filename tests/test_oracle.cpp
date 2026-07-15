#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <hibitset/hibitset.hpp>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <random>
#include <set>

namespace {

std::size_t oracle_next(const std::set<std::size_t>& ref, std::size_t i, std::size_t npos) {
    auto it = ref.lower_bound(i); 
    return it == ref.end() ? npos : *it;
}

std::size_t oracle_prev(const std::set<std::size_t>& ref, std::size_t i, std::size_t npos) {
    auto it = ref.upper_bound(i);
    return it == ref.begin() ? npos : *std::prev(it);
}   

template<std::size_t U>
bool states_agree(const hibitset::bitset<U>& bitset, const std::set<std::size_t>& ref) {
    if (bitset.count() != ref.size()) return false;

    // Forward sweep via next_set
    // Test if both iterators traverse the same set bits
    auto ref_iter = ref.begin();
    for (std::size_t i = bitset.first(); i != bitset.npos;
        i = (i + 1 < U ? bitset.next_set(i + 1) : bitset.npos), ++ref_iter) {
        if (ref_iter == ref.end() || *ref_iter != i) return false;
    }
    if (ref_iter != ref.end()) return false;

    // Backward sweep via prev_set
    // Test if both iterators traverse the same set bits in reverse 
    auto ref_iter_ = ref.rbegin();
    for (std::size_t i = bitset.last(); i != bitset.npos;
        i = (i > 0 ? bitset.prev_set(i - 1) : bitset.npos), ++ref_iter_) {
        if (ref_iter_ == ref.rend() || *ref_iter_ != i) return false;
    }
    if (ref_iter_ != ref.rend()) return false;

    // Test if the projected ranges of the bitset and the reference set are equal
    return std::ranges::equal(bitset, ref);
}

}

TEMPLATE_TEST_CASE_SIG("random operations agree with a std::set oracle", "[bitset][fuzz]",
    ((std::size_t U), U), 1, 2, 63, 64, 65, 100, 128, 4095, 4096, 4097, 262144) {
    hibitset::bitset<U> a, b;
    std::set<std::size_t> ref_a, ref_b;

    std::mt19937 rng(42);
    std::uniform_int_distribution<std::size_t> idx_dist(0, U - 1);
    std::uniform_int_distribution<int> op_dist(0, 4);

    // Compare next_set/prev_set at a few random points against the oracle
    auto check_queries = [&](const hibitset::bitset<U>& bits, const std::set<std::size_t>& ref) {
        for (int q = 0; q < 3; ++q) {
            std::size_t query = idx_dist(rng);
            REQUIRE(bits.next_set(query) == oracle_next(ref, query, bits.npos));
            REQUIRE(bits.prev_set(query) == oracle_prev(ref, query, bits.npos));
        }
    };

    // Randomly set or clear a bit and check the return value against the oracle
    auto mutate = [&](hibitset::bitset<U>& bits, std::set<std::size_t>& ref) {
        std::size_t idx = idx_dist(rng);
        if (std::uniform_int_distribution<int>(0, 1)(rng)) {
            bool inserted = ref.insert(idx).second;
            REQUIRE(bits.set(idx) == inserted);
        } else {
            bool erased = ref.erase(idx) > 0;
            REQUIRE(bits.clear(idx) == erased);
        }
        check_queries(bits, ref);
    };

    for (int i = 0; i < 1'000; ++i) {
        switch (op_dist(rng)) {
            case 0:
                mutate(a, ref_a);
                break;
            case 1:
                mutate(b, ref_b);
                break;
            case 2:
                a &= b;
                std::erase_if(ref_a, [&](std::size_t x) { return !ref_b.contains(x); });
                check_queries(a, ref_a);
                break;
            case 3:
                a |= b;
                ref_a.insert(ref_b.begin(), ref_b.end());
                check_queries(a, ref_a);
                break;
            case 4: {
                std::set<std::size_t> xor_result;
                std::ranges::set_symmetric_difference(ref_a, ref_b,
                    std::inserter(xor_result, xor_result.end()));
                a ^= b;
                ref_a = std::move(xor_result);
                check_queries(a, ref_a);
                break;
            }
        }

        REQUIRE(states_agree(a, ref_a));
        REQUIRE(states_agree(b, ref_b));

    }
}