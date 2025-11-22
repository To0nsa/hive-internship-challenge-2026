#pragma once

#include "collision/Collider.h"

#include <SFML/Graphics/Rect.hpp>
#include <optional>

namespace collision {

    inline std::optional<sf::FloatRect> overlap(const Collider& a, const Collider& b) {
        return a.worldAabb().findIntersection(b.worldAabb());
    }

    inline bool intersects(const Collider& a, const Collider& b) {
        return overlap(a, b).has_value();
    }

} // namespace collision