#pragma once

#include <array>
#include <bit>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <random>


constexpr int SERVER_RANK = 1;
constexpr int CLIENT_RANK = 0;

using Key = uint64_t;
using Mapped = std::array<std::byte, 150>;
using ValueType = std::pair<const Key, Mapped>;

constexpr size_t NumElements = 13421773;  // 2GB
constexpr size_t NIteration = 10000;

enum BlockingMode : int {
    None = 0,
    DepthFirst,
    vEB
};

inline Key FNV_hash(uint64_t x)
{
    const auto bytes = std::bit_cast<std::array<uint8_t, sizeof(uint64_t)>>(x);

    uint64_t hash = 14695981039346656037ul;
    for (const auto byte : bytes) {
        hash ^= byte;
        hash *= 1099511628211ul;
    }

    return static_cast<Key>(hash);
}
template <class URBG>
Mapped random_bytes(URBG&& engine)
{
    static std::uniform_int_distribution<unsigned char> dist;
    Mapped result;
    for (auto& byte : result) {
        byte = static_cast<std::byte>(dist(engine));
    }
    return result;
}

template <class URBG, class MapType>
inline std::chrono::nanoseconds construct(URBG& prng, MapType& map)
{
    const auto begin_time = std::chrono::high_resolution_clock::now();
    for (uint64_t i{NumElements}; i != 0; i--) {
        map.insert({FNV_hash(i - 1), random_bytes(prng)});
    }
    const auto end_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - begin_time);
}
