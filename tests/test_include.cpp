#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <hibitset/hibitset.hpp>
#include <utility>
#include <type_traits>
#include <ranges>

namespace {

TEST_CASE("can include the header", "[hibitset]") {}

TEMPLATE_TEST_CASE_SIG("can correctly initialize a bitset", "[bitset][template]",
    ((std::size_t U), U), 1, 2, 63, 64, 65, 100, 128, 4095, 4096, 4097, 262144) {
    hibitset::bitset<U> bits;
    REQUIRE(bits.npos == U);
    REQUIRE(bits.capacity() == U);
    REQUIRE(bits.count() == 0);
    REQUIRE(bits.empty());
    REQUIRE(!bits.all());
    REQUIRE(!bits.any());
    REQUIRE(bits.first() == hibitset::bitset<U>::npos);
    REQUIRE(bits.last() == hibitset::bitset<U>::npos);
    REQUIRE(bits.begin() == bits.end());
}

static_assert(std::forward_iterator<hibitset::bitset<64>::const_iterator>);
static_assert(std::ranges::forward_range<hibitset::bitset<64>>);
static_assert(std::is_trivially_copyable_v<hibitset::bitset<64>>);

}