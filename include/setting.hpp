#pragma once

#include "setting_basis.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <random>


constexpr double ZipfSkewness = Zipf_skewness;
constexpr double UpdateRatio = Update_ratio;

template <class UIntType>
struct Zipf_distribution {
    using result_type = UIntType;

    static_assert(NumElements > 0);
    static_assert(ZipfSkewness > 0);

    constexpr static double h_integral(double x)
    {
        using std::log;
        const double log_x = log(x);
        return helper2((1.0 - ZipfSkewness) * log_x) * log_x;
    }
    constexpr static double h(double x)
    {
        using std::exp;
        using std::log;
        return exp(-ZipfSkewness * log(x));
    }
    constexpr static double h_integral_inv(double x)
    {
        using std::exp;
        using std::max;
        return exp(helper1(max({-1.0, x * (1 - ZipfSkewness)})) * x);
    }
    constexpr static double helper1(double x)
    {
        using std::abs;
        using std::log1p;
        if (abs(x) > 1e-8) {
            return log1p(x) / x;
        } else {
            return 1.0 - x * (0.5 - x * (1.0 / 3 - x / 4));
        }
    }
    constexpr static double helper2(double x)
    {
        using std::abs;
        using std::expm1;
        if (abs(x) > 1e-8) {
            return expm1(x) / x;
        } else {
            return 1.0 + x / 2 * (1.0 + x / 3 * (1.0 + x / 4));
        }
    }

    static constexpr double h_integral_x1 = h_integral(1.5) - 1.0;
    static constexpr double h_integral_num_elements = h_integral(double{NumElements} + 0.5);
    static constexpr double s = 2.0 - h_integral_inv(h_integral(2.5) - h(2.0));

private:
    inline static std::uniform_real_distribution<double> real_distribution{-h_integral_num_elements, -h_integral_x1};

public:
    template <class URBG>
    constexpr result_type operator()(URBG& g) const
    {
        using std::clamp;
        using std::max;
        for (;;) {
            const double u = -real_distribution(g);
            const double x = h_integral_inv(u);
            const UIntType k = clamp<UIntType>(static_cast<UIntType>(x), 1, NumElements);
            const double k_double = static_cast<double>(k);

            if (k_double - x <= s || u >= h_integral(k_double + 0.5) - h(k_double)) {
                return UIntType{k - 1};
            }
        }
    }
};
enum ReadOrUpdate {
    Read,
    Update
};
template <class URBG>
ReadOrUpdate generate_task(URBG&& engine)
{
    static std::bernoulli_distribution dist{UpdateRatio};
    return dist(engine) ? Update : Read;
}

void read(Mapped);

template <class URBG, class MapType>
inline std::chrono::nanoseconds search(URBG& prng, MapType& map)
{
    constexpr Zipf_distribution<uint64_t> zipf;

    const auto begin_time = std::chrono::high_resolution_clock::now();
    for (auto i = NIteration; i != 0; i--) {
        Key key = FNV_hash(zipf(prng));
        if (generate_task(prng) == Update) {
            map.find(key)->second = random_bytes(prng);
        } else {
            read(map.find(key)->second);
        }
    }
    const auto end_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - begin_time);
}

template <class URBG, class MapType>
inline std::chrono::nanoseconds range_query(URBG& prng, MapType& map)
{
    constexpr Zipf_distribution<uint64_t> zipf;
    std::uniform_int_distribution<size_t> uniform{1, 100};

    const auto begin_time = std::chrono::high_resolution_clock::now();
    for (auto i = NIteration; i != 0; i--) {
        Key key = FNV_hash(zipf(prng));
        if (generate_task(prng) == Update) {
            map.find(key)->second = random_bytes(prng);
        } else {
            const auto n_read = uniform(prng);
            auto iter = map.find(key);
            for (size_t i = 0; i < n_read && iter != map.end(); i++, iter++) {
                read(iter->second);
            }
        }
    }
    const auto end_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - begin_time);
}

template <class MapType>
inline std::array<std::chrono::nanoseconds, 2> benchmark(MapType& map)
{
    std::mt19937 prng;

    std::array<std::chrono::nanoseconds, 2> result;
    result[0] = construct(prng, map);
    result[1] = search(prng, map);

    return result;
}
