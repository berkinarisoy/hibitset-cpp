#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <hibitset/hibitset.hpp>
#include <cstddef>

namespace {

template<std::size_t U>
consteval bool check_set_clear() {
    hibitset::bitset<U> bits;

    // Test setting and clearing bits at an index
    auto touch = [&bits](std::size_t idx) consteval -> bool {
        if (bits.test(idx)) return false;
        if (!bits.set(idx)) return false;
        if (!bits.test(idx)) return false;
        if (bits.set(idx)) return false;   
        if (!bits.clear(idx)) return false;
        if (bits.test(idx)) return false;
        if (bits.clear(idx)) return false;
        return true;
    };

    // Test the first and last indices
    if (!touch(0)) return false;
    if (!touch(U - 1)) return false;

    // Test the first/second layer boundary
    if constexpr (U > 64) {
        if (!touch(63)) return false;
        if (!touch(64)) return false;
    }

    // Test the second/third layer boundary
    if constexpr (U > 4096) {
        if (!touch(4095)) return false;
        if (!touch(4096)) return false;
    }

    return true;
}

template<std::size_t U>
consteval bool check_next_prev() {
    hibitset::bitset<U> bits;

    // Test next_set and prev_set on an empty bitset
    if (bits.next_set(0) != bits.npos) return false;
    if (bits.prev_set(U - 1) != bits.npos) return false;

    // Test next_set and prev_set after setting the first bit
    bits.set(0);
    if (bits.next_set(0) != 0) return false;
    if (bits.prev_set(U - 1) != 0) return false;

    // Test next_set and prev_set after setting the last bit
    if constexpr (U > 1) {
        bits.set(U - 1);
        if (bits.next_set(1) != U - 1) return false;
        if (bits.prev_set(U - 2) != 0) return false;
    }

    return true;
}

template<std::size_t U>
consteval bool check_algebra() {
    hibitset::bitset<U> a;
    hibitset::bitset<U> b;

    a.set(0);
    b.set(0);
    if constexpr (U > 1) a.set(U - 1);

    // Test bitwise AND, OR, XOR
    hibitset::bitset<U> and_result = a & b;
    if (and_result.count() != 1 || !and_result.test(0)) return false;

    hibitset::bitset<U> or_result = a | b;
    if (!(or_result == a)) return false;

    hibitset::bitset<U> xor_result = a ^ b;
    if constexpr (U > 1) {
        if (xor_result.count() != 1 || !xor_result.test(U - 1)) return false;
    } else {
        if (!xor_result.empty()) return false;
    }

    // Test the assignment operators
    hibitset::bitset<U> c = a;
    c &= b;
    if (!(c == and_result)) return false;

    c = a;
    c |= b;
    if (!(c == or_result)) return false;

    c = a;
    c ^= b;
    if (!(c == xor_result)) return false;

    // Test the second layer boundary for larger bitsets
    if constexpr (U >= 4096) {
        hibitset::bitset<U> a;
        hibitset::bitset<U> b;
        // a: 0...0 0...001 0...0 0...0 ...
        // b: 0...0 0...010 0...0 0...0 ...
        a.set(64);
        b.set(65);
        a &= b;
        if (!a.empty()) return false;
        if (a.next_set(0) != a.npos) return false;
        if (a.prev_set(4095) != a.npos) return false;
        a.reset();
        b.reset();

        // a: 0...0 0...01 0...0 0...0 ...
        // b: 0...0 0...01 0...0 0...0 ...
        a.set(64);
        b.set(64);
        a &= b;
        if (a.count() != 1) return false;
        if (!a.test(64)) return false;
        if (a.next_set(0) != 64) return false;
        if (a.prev_set(4095) != 64) return false;
        a.reset();
        b.reset();
    }

    // Test the third layer boundary for even larger bitsets
    if constexpr (U >= 262144) {
        hibitset::bitset<U> a;
        hibitset::bitset<U> b;
        // a:
        // 1 --------------- 0 ---------------
        // 1 ---- 0 ---- ... 0 ---- 0 ---- ...
        // 0...01 00...0 ... 00...0 00...0 ...
        // b:
        // 0 --------------- 1 ---------------
        // 0 ---- 0 ---- ... 1 ---- 0 ---- ...
        // 00...0 0...00 ... 0...01 00...0 ...
        a.set(0);
        b.set(4096);
        a &= b;
        if (!a.empty()) return false;
        if (a.next_set(0) != a.npos) return false;
        if (a.prev_set(262143) != a.npos) return false;
    }

    return true;
}

template<std::size_t U>
consteval bool check_iteration() {
    hibitset::bitset<U> bits;
    bits.set(0);
    if constexpr (U > 1) bits.set(U - 1);

    // Test if the iterator traverses in ascending order 
    // and visits all set bits exactly once
    std::size_t count = 0;
    std::size_t last_seen = 0;
    bool first_iter = true;
    for (std::size_t idx : bits) {
        if (!first_iter && idx <= last_seen) return false;
        last_seen = idx;
        first_iter = false;
        ++count;
    }
    if (count != (U > 1 ? 2 : 1)) return false;

    auto it = bits.begin();
    if (*it != 0) return false;
    ++it;
    if constexpr (U > 1) {
        if (*it != U - 1) return false;
        ++it;
    }
    if (!(it == bits.end())) return false;

    return true;
}

TEMPLATE_TEST_CASE_SIG("can check set/clear in compile-time", "[constexpr]",
    ((std::size_t U), U), 1, 63, 64, 65, 4095, 4096, 4097, 262144) {
    STATIC_REQUIRE(check_set_clear<U>());
}

TEMPLATE_TEST_CASE_SIG("can check next_set/prev_set in compile-time", "[constexpr]",
    ((std::size_t U), U), 1, 63, 64, 65, 4095, 4096, 4097, 262144) {
    STATIC_REQUIRE(check_next_prev<U>());
}

TEMPLATE_TEST_CASE_SIG("can check bitwise algebra in compile-time", "[constexpr]",
    ((std::size_t U), U), 1, 63, 64, 65, 4095, 4096, 4097, 262144) {
    STATIC_REQUIRE(check_algebra<U>());
}

TEMPLATE_TEST_CASE_SIG("can check iteration in compile-time", "[constexpr]",
    ((std::size_t U), U), 1, 63, 64, 65, 4095, 4096, 4097, 262144) {
    STATIC_REQUIRE(check_iteration<U>());
}

}