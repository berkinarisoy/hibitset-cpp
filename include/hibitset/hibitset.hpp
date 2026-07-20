#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <bit>
#include <cassert>
#include <iterator>

namespace hibitset {

/// @brief A fixed-size hierarchical bitset implementation for efficient bit manipulation.
/// @tparam U The size of the universe.
template<std::size_t U>
class bitset {
private:
    static_assert(U > 0, "hibitset: Universe size must be positive");

    static constexpr std::size_t get_layer_count(std::size_t universe_size) {
        if (universe_size == 0) return 0;
        if (universe_size < 64) return 1;
        return (std::bit_width(universe_size) - 1) / 6;
    }

    template<std::size_t L>
    static constexpr std::array<std::size_t, L> get_layer_sizes() {
        std::array<std::size_t, L> sizes {};
        std::size_t temp {U};
        for (std::size_t i = 0; i < L; ++i) {
            sizes[i] =  temp / 64 + (temp % 64 != 0);
            temp = sizes[i];
        }
        return sizes;
    }

    template<std::size_t L>
    static constexpr std::array<std::size_t, L> get_prefix_sums(const std::array<std::size_t, L>& layer_sizes) {
        std::array<std::size_t, L> prefix_sums {};
        std::size_t temp {0};
        for (std::size_t i = 0; i < L; ++i) {
            prefix_sums[i] = temp;
            temp += layer_sizes[i];
        }
        return prefix_sums;
    }

    template<std::size_t L>
    static constexpr std::size_t total_chunks(const std::array<std::size_t, L>& layer_sizes) {
        std::size_t total {0};
        for (std::size_t i = 0; i < L; ++i)
            total += layer_sizes[i];
        return total;
    }

    static constexpr std::size_t layer_count = get_layer_count(U);
    static constexpr auto layer_sizes = get_layer_sizes<layer_count>();
    static constexpr auto layer_offsets = get_prefix_sums(layer_sizes);
    // bits >= U in the last chunk are always zero as an invariant
    std::array<std::uint64_t, total_chunks(layer_sizes)> buffer {};
    std::size_t size_ {0};

    static constexpr void check_range(std::size_t idx) {
        assert(idx < U && "hibitset: Index out of range");
    }

    /// @brief Recounts the number of set bits in the bitset.
    constexpr void recount() {
        size_ = 0;
        for (std::size_t i = 0; i < layer_sizes[0]; ++i)
            size_ += std::popcount(buffer[i]);
    }

    /// @brief Rebuilds the summary layers based on the lowest layer.
    constexpr void rebuild_summaries() {
        // Clear all summary layers
        for (std::size_t i = layer_sizes[0]; i < buffer.size(); ++i)
            buffer[i] = 0;

        // Rebuild summary layers based on the lowest layer
        for (std::size_t i = 0; i + 1 < layer_count; ++i)
            for (std::size_t j = 0; j < layer_sizes[i]; ++j)
                if (buffer[layer_offsets[i] + j] != 0)
                    buffer[layer_offsets[i + 1] + (j >> 6)] |= 1ULL << (j & 63);
    }

public:
    static constexpr std::size_t npos = U;

    constexpr bitset() = default;

    /// @brief Tests if the bit at the given index is set.
    /// @param idx The index to test.
    /// @return True if the bit is set, false otherwise.
    [[nodiscard]] constexpr bool test(std::size_t idx) const noexcept {
        check_range(idx);
        return (buffer[idx >> 6] >> (idx & 63)) & 1;
    }

    /// @brief Sets the bit at the given index.
    /// @param idx The index to set.
    /// @return False if the bit was already set, true otherwise.
    constexpr bool set(std::size_t idx) noexcept {
        check_range(idx);

        for (std::size_t i = 0; i < layer_count; ++i) {
            std::size_t chunk_idx = layer_offsets[i] + (idx >> (6 * (i + 1)));
            std::uint64_t bit = 1ULL << ((idx >> (6 * i)) & 63);
            std::uint64_t prev = buffer[chunk_idx];

            // The bit was already set
            if (i == 0 && (prev & bit)) return false;

            buffer[chunk_idx] = prev | bit;

            // Early break if the chunk was already set
            if (prev != 0) break;
        }
        ++size_;
        return true;
    }

    /// @brief Clears the bit at the given index.
    /// @param idx The index to clear.
    /// @return False if the bit was already zero, true otherwise.
    constexpr bool clear(std::size_t idx) noexcept {
        check_range(idx);

        // The bit was already zero
        if (!test(idx))
            return false;

        for (std::size_t i = 0; i < layer_count; ++i) {
            std::size_t chunk_idx = layer_offsets[i] + (idx >> (6 * (i + 1)));
            std::uint64_t bit = 1ULL << ((idx >> (6 * i)) & 63);
            buffer[chunk_idx] &= ~bit;

            // Stop propagating once the word still has other bits set
            if (buffer[chunk_idx] != 0) break;
        }
        --size_;
        return true;
    }

    /// @brief Finds the next set bit at or after the given index.
    /// @param idx The index to start searching from.
    /// @return The index of the next set bit, or npos if not found.
    [[nodiscard]] constexpr std::size_t next_set(std::size_t idx) const noexcept {
        check_range(idx);

        // Iterate through the layers, starting from the lowest layer
        for (std::size_t i = 0; i < layer_count; ++i) {
            std::size_t local_chunk_idx = idx >> (6 * (i + 1));
            std::size_t bit = (idx >> (6 * i)) & 63;

            // Don't include the chunk that contains the index
            // after the first layer, since it is already handled
            std::uint64_t chunk = buffer[layer_offsets[i] + local_chunk_idx] &
                ((i == 0 ? ~0ULL : ~1ULL) << bit);

            if (chunk) {
                // Traverse down the layers to find the exact index
                for (std::size_t j = i; j-- > 0;) {
                    local_chunk_idx = (local_chunk_idx << 6) + std::countr_zero(chunk);
                    chunk = buffer[layer_offsets[j] + local_chunk_idx];
                }
                return (local_chunk_idx << 6) + std::countr_zero(chunk);
            }
        }

        // Perform a linear search in the top layer
        for (std::size_t i = (idx >> (6 * layer_count)) + 1; i < layer_sizes[layer_count - 1]; ++i) {
            std::uint64_t chunk = buffer[layer_offsets[layer_count - 1] + i];

            if (chunk) {
                // Traverse down the layers to find the exact index
                std::size_t local_chunk_idx = i;
                for (std::size_t j = layer_count - 1; j-- > 0;) {
                    local_chunk_idx = (local_chunk_idx << 6) + std::countr_zero(chunk);
                    chunk = buffer[layer_offsets[j] + local_chunk_idx];
                }
                return (local_chunk_idx << 6) + std::countr_zero(chunk);
            }
        }

        return npos;
    }

    /// @brief Finds the previous set bit at or before the given index.
    /// @param idx The index to start searching from.
    /// @return The index of the previous set bit, or npos if not found.
    [[nodiscard]] constexpr std::size_t prev_set(std::size_t idx) const noexcept {
        check_range(idx);

        // Iterate through the layers, starting from the lowest layer
        for (std::size_t i = 0; i < layer_count; ++i) {
            std::size_t local_chunk_idx = idx >> (6 * (i + 1));
            std::size_t bit = (idx >> (6 * i)) & 63;

            // Don't include the chunk that contains the index
            // after the first layer, since it is already handled
            std::uint64_t chunk = buffer[layer_offsets[i] + local_chunk_idx] &
                ((i == 0 ? ~0ULL : ~(1ULL << 63)) >> (63 - bit));

            if (chunk) {
                // Traverse down the layers to find the exact index
                for (std::size_t j = i; j-- > 0;) {
                    local_chunk_idx = (local_chunk_idx << 6) + (63 - std::countl_zero(chunk));
                    chunk = buffer[layer_offsets[j] + local_chunk_idx];
                }
                return (local_chunk_idx << 6) + (63 - std::countl_zero(chunk));
            }
        }

        // Perform a linear search in the top layer, going backwards
        for (std::size_t i = idx >> (6 * layer_count); i-- > 0;) {
            std::uint64_t chunk = buffer[layer_offsets[layer_count - 1] + i];

            if (chunk) {
                // Traverse down the layers to find the exact index
                std::size_t local_chunk_idx = i;
                for (std::size_t j = layer_count - 1; j-- > 0;) {
                    local_chunk_idx = (local_chunk_idx << 6) + (63 - std::countl_zero(chunk));
                    chunk = buffer[layer_offsets[j] + local_chunk_idx];
                }
                return (local_chunk_idx << 6) + (63 - std::countl_zero(chunk));
            }
        }

        return npos;
    }

    /// @return The maximum number of bits that can be stored.
    [[nodiscard]] constexpr std::size_t capacity() const noexcept { return U; }

    /// @return The number of bits currently set in the bitset.
    [[nodiscard]] constexpr std::size_t count() const noexcept { return size_; }

    /// @return True if no bits are set, false otherwise.
    [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }

    /// @return True if all bits are set, false otherwise.
    [[nodiscard]] constexpr bool all() const noexcept { return size_ == U; }

    /// @return True if any bits are set, false otherwise.
    [[nodiscard]] constexpr bool any() const noexcept { return size_ > 0; }

    /// @brief Finds the first set bit.
    /// @return The index of the first set bit, or npos if not found.
    [[nodiscard]] constexpr std::size_t first() const noexcept { return next_set(0); }

    /// @brief Finds the last set bit.
    /// @return The index of the last set bit, or npos if not found.
    [[nodiscard]] constexpr std::size_t last() const noexcept { return prev_set(U - 1); }

    /// @brief Resets the bitset, clearing all bits and setting the count to zero.
    constexpr void reset() {
        buffer.fill(0);
        size_ = 0;
    }

    /// @brief Compares this bitset with another for equality.
    [[nodiscard]] constexpr bool operator==(const bitset& other) const noexcept {
        if (size_ != other.size_) return false;
        for (std::size_t i = 0; i < layer_sizes[0]; ++i) {
            if (buffer[i] != other.buffer[i]) return false;
        }
        return true;
    }

    /// @brief Performs a bitwise OR operation with another bitset.
    /// @return A reference to this bitset after the operation.
    constexpr bitset& operator|=(const bitset& other) noexcept {
        for (std::size_t i = 0; i < buffer.size(); ++i)
            buffer[i] |= other.buffer[i];

        recount();
        return *this;
    }

    /// @brief Performs a bitwise AND operation with another bitset.
    /// @return A reference to this bitset after the operation.
    constexpr bitset& operator&=(const bitset& other) noexcept {
        for (std::size_t i = 0; i < layer_sizes[0]; ++i)
            buffer[i] &= other.buffer[i];

        rebuild_summaries();
        recount();
        return *this;
    }

    /// @brief Performs a bitwise XOR operation with another bitset.
    /// @return A reference to this bitset after the operation.
    constexpr bitset& operator^=(const bitset& other) noexcept {
        for (std::size_t i = 0; i < layer_sizes[0]; ++i)
            buffer[i] ^= other.buffer[i];

        rebuild_summaries();
        recount();
        return *this;
    }

    /// @brief Returns the bitwise OR (set union) of this bitset and another.
    /// @return A new bitset containing the result.
    [[nodiscard]] friend constexpr bitset operator|(bitset lhs, const bitset& rhs) noexcept {
        lhs |= rhs;
        return lhs;
    }

    /// @brief Returns the bitwise AND (set intersection) of this bitset and another.
    /// @return A new bitset containing the result.
    [[nodiscard]] friend constexpr bitset operator&(bitset lhs, const bitset& rhs) noexcept {
        lhs &= rhs;
        return lhs;
    }

    /// @brief Returns the bitwise XOR (symmetric difference) of this bitset and another.
    /// @return A new bitset containing the result.
    [[nodiscard]] friend constexpr bitset operator^(bitset lhs, const bitset& rhs) noexcept {
        lhs ^= rhs;
        return lhs;
    }

    /// @brief A forward iterator that traverses the set bits from the start.
    class const_iterator {
    private:
        const bitset* parent_ = nullptr;
        std::size_t pos_ = npos;

        constexpr const_iterator(
            const bitset* parent,
            std::size_t pos
        ) noexcept : parent_(parent), pos_(pos) {}

        friend class bitset;
    public:
        using iterator_concept  = std::forward_iterator_tag;
        using iterator_category = std::forward_iterator_tag;
        using value_type        = std::size_t;
        using difference_type   = std::ptrdiff_t;
        using reference         = std::size_t;
        using pointer           = void;

        constexpr const_iterator() noexcept = default;

        constexpr std::size_t operator*() const noexcept { return pos_; }

        // Pre-increment
        constexpr const_iterator& operator++() noexcept {
            pos_ = (pos_ + 1 < U) ? parent_->next_set(pos_ + 1) : npos;
            return *this;
        }

        // Post-increment
        constexpr const_iterator operator++(int) noexcept {
            const_iterator old = *this;
            ++(*this);
            return old;
        }

        friend constexpr bool operator==(
            const const_iterator&,
            const const_iterator&
        ) noexcept = default;
    };

    [[nodiscard]] constexpr const_iterator begin() const noexcept { return const_iterator(this, first()); }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return const_iterator(this, npos); }
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }
    [[nodiscard]] constexpr const_iterator cend() const noexcept { return end(); }
};
}
