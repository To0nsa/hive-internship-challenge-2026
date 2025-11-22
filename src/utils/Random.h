#pragma once

#include <SFML/System/Vector2.hpp>
#include <chrono>
#include <cstdint>
#include <random>
#include <vector>

namespace Random {

    using Engine = std::mt19937;

    inline Engine& engine() {
        static Engine eng = [] {
            std::random_device rd;
            return Engine{rd()};
        }();
        return eng;
    }

    inline void seed(std::uint64_t s) { engine().seed(static_cast<std::uint32_t>(s)); }

    inline std::uint64_t timeSeed() {
        using namespace std::chrono;
        return static_cast<std::uint64_t>(
            duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count());
    }

    inline int rangei(int minInclusive, int maxInclusive) {
        std::uniform_int_distribution<int> dist(minInclusive, maxInclusive);
        return dist(engine());
    }

    inline float rangef(float minInclusive, float maxInclusive) {
        std::uniform_real_distribution<float> dist(minInclusive, maxInclusive);
        return dist(engine());
    }

    inline bool chance(float probability01) {
        if (probability01 <= 0.f)
            return false;
        if (probability01 >= 1.f)
            return true;
        std::bernoulli_distribution dist(probability01);
        return dist(engine());
    }

    template <typename T> inline const T& pick(const std::vector<T>& v) {
        return v[static_cast<std::size_t>(rangei(0, static_cast<int>(v.size() - 1)))];
    }

} // namespace Random
