#pragma once
#include "Math.h"

#include <SFML/Graphics/Rect.hpp>

namespace geom {

    inline float left(const sf::FloatRect& r) { return r.position.x; }
    inline float top(const sf::FloatRect& r) { return r.position.y; }
    inline float width(const sf::FloatRect& r) { return r.size.x; }
    inline float height(const sf::FloatRect& r) { return r.size.y; }

    inline float right(const sf::FloatRect& r) { return left(r) + width(r); }
    inline float bottom(const sf::FloatRect& r) { return top(r) + height(r); }

    inline bool overlap1D(float aL, float aR, float bL, float bR) { return (aR > bL) && (aL < bR); }

    inline bool aabbIntersects(const sf::FloatRect& a, const sf::FloatRect& b) {
        return a.findIntersection(b).has_value();
    }

    inline bool aabbIntersects(const sf::FloatRect& a, const sf::FloatRect& b, sf::FloatRect& out) {
        if (auto inter = a.findIntersection(b)) {
            out = *inter;
            return true;
        }
        return false;
    }

    inline bool touchTop(const sf::FloatRect& a, const sf::FloatRect& b, float eps = 1.0f) {
        return math::nearEq(bottom(a), top(b), eps) &&
               overlap1D(left(a), right(a), left(b), right(b));
    }

    inline bool touchSide(const sf::FloatRect& a, const sf::FloatRect& b, float eps = 1.0f) {
        const bool leftTouch  = math::nearEq(right(a), left(b), eps);
        const bool rightTouch = math::nearEq(left(a), right(b), eps);
        return (leftTouch || rightTouch) && overlap1D(top(a), bottom(a), top(b), bottom(b));
    }

} // namespace geom
