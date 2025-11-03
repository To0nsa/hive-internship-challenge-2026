// Collider.h
#pragma once
#include <SFML/Graphics.hpp>

class Collider {
  public:
    virtual ~Collider() = default;

    virtual sf::FloatRect worldAabb() const = 0;

    bool intersects(const Collider& other) const {
        return worldAabb().findIntersection(other.worldAabb()).has_value();
    }
};
