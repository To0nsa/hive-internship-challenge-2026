#pragma once
#include "utils/Math.h"

#include <SFML/Graphics/Rect.hpp>

namespace geom {

    // Basic accessors for SFML 3 FloatRect, kept for readability and
    // to avoid repeating r.position.x / r.size.y everywhere.
    inline float left(const sf::FloatRect& r) { return r.position.x; }
    inline float top(const sf::FloatRect& r) { return r.position.y; }
    inline float width(const sf::FloatRect& r) { return r.size.x; }
    inline float height(const sf::FloatRect& r) { return r.size.y; }

    // Derived edges, built from the accessors above.
    // Used by collision helpers (touchTop / touchSide).
    inline float right(const sf::FloatRect& r) { return left(r) + width(r); }
    inline float bottom(const sf::FloatRect& r) { return top(r) + height(r); }

    // 1-D interval overlap test, used by the AABB “touch” checks.
    // Returns true when [aL,aR] and [bL,bR] overlap on a single axis.
    inline bool overlap1D(float aL, float aR, float bL, float bR) { return (aR > bL) && (aL < bR); }

    // Basic AABB intersection using SFML’s helper.
    // Used when you just need “do these two rectangles intersect?”
    inline bool aabbIntersects(const sf::FloatRect& a, const sf::FloatRect& b) {
        return a.findIntersection(b).has_value();
    }

    // Same as above but also returns the intersection rectangle in `out`.
    // Used where you need the overlap area (e.g. for more detailed collision response).
    inline bool aabbIntersects(const sf::FloatRect& a, const sf::FloatRect& b, sf::FloatRect& out) {
        if (auto inter = a.findIntersection(b)) {
            out = *inter;
            return true;
        }
        return false;
    }

    // Returns true if A is (approximately) resting on top of B.
    //
    // Project usage:
    // - Ground / obstacle checks (e.g. “is the player standing on this collider?”)
    //
    // Logic:
    // - bottom(A) is ~ equal to top(B) on Y (with epsilon, via math::nearEq)
    // - and they overlap on X (using overlap1D).
    inline bool touchTop(const sf::FloatRect& a, const sf::FloatRect& b, float eps = 1.0f) {
        return math::nearEq(bottom(a), top(b), eps) &&
               overlap1D(left(a), right(a), left(b), right(b));
    }

    // Returns true if A is touching B on the left or right side.
    //
    // Project usage:
    // - Side collision checks (e.g. player hitting obstacles from the side).
    //
    // Logic:
    // - Either right(A) ≈ left(B) or left(A) ≈ right(B) on X (with epsilon)
    // - and they overlap on Y (using overlap1D).
    inline bool touchSide(const sf::FloatRect& a, const sf::FloatRect& b, float eps = 1.0f) {
        const bool leftTouch  = math::nearEq(right(a), left(b), eps);
        const bool rightTouch = math::nearEq(left(a), right(b), eps);
        return (leftTouch || rightTouch) && overlap1D(top(a), bottom(a), top(b), bottom(b));
    }

} // namespace geom
