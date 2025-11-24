#pragma once
#include <SFML/System/Vector2.hpp>
#include <algorithm> // std::clamp
#include <cmath>     // std::expm1, std::abs
#include <cstdint>   // std::uint32_t

namespace math {

    // Exponential smoothing factor for frame-rate–independent easing.
    //
    // Usage in this project:
    // - World::update(): used to compute the "alpha" when easing:
    //   * the camera target X toward the player (kTargetCatchupLerp)
    //   * the actual camera center X toward the target (kCatchupLerp)
    //
    // Intuition:
    // - lambda: responsiveness (bigger => snappier).
    // - dt: frame time.
    // - Return value in [0,1] suitable for: new = old + alpha * (target - old).
    inline float expSmoothingFactor(float lambda, float dt) {
        if (dt <= 0.f || lambda <= 0.f)
            return 0.f;
        const float a = -std::expm1(-lambda * dt);
        return std::clamp(a, 0.f, 1.f);
    }

    // Approximate float comparison with an epsilon.
    //
    // Usage in this project:
    // - utils/Geom.h:
    //   * geom::touchTop() and geom::touchSide() use nearEq() to treat
    //     "almost touching" AABBs as touching, despite float error.
    // - Indirectly used in:
    //   * World::update(): DPS when the player stands on top of an obstacle
    //     via geom::touchTop().
    //   * Actor::applyPhysics(): detecting when the actor is resting on
    //     top of ground colliders (via geom helpers).
    //
    // Purpose:
    // - Avoid flaky collisions due to tiny float discrepancies when objects
    //   are supposed to rest exactly on top of each other.
    inline bool nearEq(float a, float b, float eps = 1e-5f) { return std::abs(a - b) <= eps; }

    // Clamp x into [0, 1].
    //
    // Usage in this project:
    // - Currently only used inside math::remap().
    //
    // Purpose:
    // - Generic helper for normalizing values into the [0,1] range, useful
    //   for interpolation, UI ratios, and tunable parameters.
    template <typename T> inline T saturate(T x) {
        return std::clamp(x, static_cast<T>(0), static_cast<T>(1));
    }

    // Linear interpolation between a and b.
    //
    // Usage in this project:
    // - Currently only used by math::remap().
    //
    // Purpose:
    // - Fundamental building block for any "blend" between two scalar values.
    //   (We use expSmoothingFactor + manual lerp in the camera logic.)
    inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

    // Inverse linear interpolation: given a range [a,b] and a value x,
    // return t such that lerp(a, b, t) == x.
    //
    // Usage in this project:
    // - Currently only used by math::remap().
    //
    // Purpose:
    // - Convert from a world-space range into a normalized [0,1] space
    //   before clamping / re-scaling.
    inline float unlerp(float a, float b, float x) {
        const float denom = (b - a);
        if (std::abs(denom) <= 1e-6f)
            return 0.f;
        return (x - a) / denom;
    }

    // Remap x from [inMin, inMax] to [outMin, outMax], with clamping.
    //
    // Typical use cases:
    // - Mapping world distances to UI progress bars.
    // - Turning raw stat ranges into 0..1 parameters for shaders / effects.
    inline float remap(float x, float inMin, float inMax, float outMin, float outMax) {
        return lerp(outMin, outMax, saturate(unlerp(inMin, inMax, x)));
    }

    // Integer division with floor semantics, even for negative values.
    //
    // Usage in this project:
    // - environment/GroundStream.h:
    //   * blockIndexForCell() calls floorDivInt(cell, kBlockCells) to
    //     map a cell index (can be negative if the view goes left of 0)
    //     to a stable block index.
    //
    // Purpose:
    // - Ensure that procedural ground blocks and their lava gaps line up
    //   consistently across the whole world, regardless of sign of the
    //   cell index.
    inline int floorDivInt(int a, int b) {
        int q = a / b;
        int r = a % b;
        if ((r != 0) && ((r > 0) != (b > 0)))
            --q;
        return q;
    }

    // Fast 32-bit integer mixing function (hash).
    //
    // Usage in this project:
    // - environment/GroundStream.h:
    //   * gapRectForBlock() uses mix32(anchor) to get a pseudo-random
    //     but deterministic gap offset per block.
    //
    // Purpose:
    // - Generate repeatable, deterministic "random" lava gap placement
    //   based solely on block index -> procedural level layout that is
    //   stable between runs.
    inline std::uint32_t mix32(std::uint32_t x) {
        x ^= x >> 16;
        x *= 0x7feb352du;
        x ^= x >> 15;
        x *= 0x846ca68bu;
        x ^= x >> 16;
        return x;
    }

    // Vector length (Euclidean norm) for sf::Vector2f.
    inline float length(const sf::Vector2f& vec) {
        return std::sqrt(vec.x * vec.x + vec.y * vec.y);
    }

    // Return a normalized vector; (0,0) if the input has zero length.
    inline sf::Vector2f normalizeVec(const sf::Vector2f& vec) {
        float len = length(vec);
        if (len > 0) {
            return vec / len;
        } else {
            return sf::Vector2f(0, 0);
        }
    }

} // namespace math
