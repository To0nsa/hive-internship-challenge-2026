#pragma once

#include <cassert>
#include <chrono>
#include <cstdint>
#include <random>
#include <vector>

namespace Random {

    // Pseudorandom engine used everywhere in the game.
    using Engine = std::mt19937;

    // Single global engine instance.
    // Default-seeded with a fixed constant => fully deterministic by default.
    inline Engine g_engine{[] {
        // Chosen arbitrary fixed seed;
        return Engine{0x12345678u};
    }()};

    // Accessor for the global engine.
    inline Engine& engine() noexcept { return g_engine; }

    // Seed the engine from a 64-bit value (we mix down to 32 bits).
    inline void seed(std::uint64_t s) {
        std::uint32_t hi    = static_cast<std::uint32_t>(s >> 32);
        std::uint32_t lo    = static_cast<std::uint32_t>(s & 0xFFFFFFFFu);
        std::uint32_t mixed = hi ^ (lo * 0x9e3779B1u); // cheap 64->32 mixing
        engine().seed(mixed);
    }

    // Time-based seed.
    inline std::uint64_t timeSeed() {
        using namespace std::chrono;
        return static_cast<std::uint64_t>(
            duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count());
    }

    // Uniform integer in [minInclusive, maxInclusive].
    inline int rangei(int minInclusive, int maxInclusive) {
        if (minInclusive > maxInclusive)
            std::swap(minInclusive, maxInclusive);
        std::uniform_int_distribution<int> dist(minInclusive, maxInclusive);
        return dist(engine());
    }

    // Uniform float in [minInclusive, maxInclusive].
    inline float rangef(float minInclusive, float maxInclusive) {
        if (minInclusive > maxInclusive)
            std::swap(minInclusive, maxInclusive);
        std::uniform_real_distribution<float> dist(minInclusive, maxInclusive);
        return dist(engine());
    }

    // Returns true with probability in [0, 1].
    inline bool chance(float probability01) {
        if (probability01 <= 0.f)
            return false;
        if (probability01 >= 1.f)
            return true;
        std::bernoulli_distribution dist(probability01);
        return dist(engine());
    }

    // Pick a random element from a non-empty vector.
    template <typename T> inline const T& pick(const std::vector<T>& v) {
        assert(!v.empty() && "Random::pick called with empty vector");
        const int idx = rangei(0, static_cast<int>(v.size() - 1));
        return v[static_cast<std::size_t>(idx)];
    }

} // namespace Random
