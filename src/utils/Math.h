#pragma once
#include <algorithm> // std::clamp
#include <cmath>     // std::expm1, std::abs
#include <cstdint>   // std::uint32_t

namespace math {
    inline float expSmoothingFactor(float lambda, float dt) {
        if (dt <= 0.f || lambda <= 0.f)
            return 0.f;
        const float a = -std::expm1(-lambda * dt);
        return std::clamp(a, 0.f, 1.f);
    }

    inline bool nearEq(float a, float b, float eps = 1e-5f) { return std::abs(a - b) <= eps; }

    template <typename T> inline T saturate(T x) {
        return std::clamp(x, static_cast<T>(0), static_cast<T>(1));
    }

    inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

    inline float unlerp(float a, float b, float x) {
        const float denom = (b - a);
        if (std::abs(denom) <= 1e-6f)
            return 0.f;
        return (x - a) / denom;
    }

    inline float remap(float x, float inMin, float inMax, float outMin, float outMax) {
        return lerp(outMin, outMax, saturate(unlerp(inMin, inMax, x)));
    }

    inline int floorDivInt(int a, int b) {
        int q = a / b;
        int r = a % b;
        if ((r != 0) && ((r > 0) != (b > 0)))
            --q;
        return q;
    }

    inline std::uint32_t mix32(std::uint32_t x) {
        x ^= x >> 16;
        x *= 0x7feb352du;
        x ^= x >> 15;
        x *= 0x846ca68bu;
        x ^= x >> 16;
        return x;
    }

    inline float length(const sf::Vector2f& vec) {
        return std::sqrt(vec.x * vec.x + vec.y * vec.y);
    }

    inline sf::Vector2f normalizeVec(const sf::Vector2f& vec) {
        float len = length(vec);
        if (len > 0) {
            return vec / len;
        } else {
            return sf::Vector2f(0, 0);
        }
    }
} // namespace math
