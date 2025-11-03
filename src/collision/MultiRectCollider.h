#pragma once

#include "Collider.h"

#include <SFML/Graphics/Rect.hpp>
#include <algorithm>
#include <limits>
#include <vector>

// A collider composed of multiple axis-aligned rectangles.
// Used for ground with gaps where a single AABB isn't sufficient.
class MultiRectCollider final : public Collider {
  public:
    MultiRectCollider() = default;

    void setRectColliders(std::vector<sf::FloatRect> rects) { m_rects = std::move(rects); }
    const std::vector<sf::FloatRect>& getRectColliders() const { return m_rects; }

    sf::FloatRect worldAabb() const override {
        if (m_rects.empty())
            return sf::FloatRect{{0.f, 0.f}, {0.f, 0.f}};

        float minX = std::numeric_limits<float>::infinity();
        float minY = std::numeric_limits<float>::infinity();
        float maxX = -std::numeric_limits<float>::infinity();
        float maxY = -std::numeric_limits<float>::infinity();

        for (const auto& r : m_rects) {
            const float l   = r.position.x;
            const float t   = r.position.y;
            const float rgt = r.position.x + r.size.x;
            const float bot = r.position.y + r.size.y;

            minX = std::min(minX, l);
            minY = std::min(minY, t);
            maxX = std::max(maxX, rgt);
            maxY = std::max(maxY, bot);
        }

        const float w = std::max(0.f, maxX - minX);
        const float h = std::max(0.f, maxY - minY);
        return sf::FloatRect{{minX, minY}, {w, h}};
    }

  private:
    std::vector<sf::FloatRect> m_rects;
};
