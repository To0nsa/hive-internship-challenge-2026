#pragma once

#include "../Entity.h"

#include <SFML/Graphics/RectangleShape.hpp>

// Minimal rotating collectible square.
class RedSquare final : public Entity {
  public:
    explicit RedSquare(const sf::Vector2f& center) : m_startCenter(center) {}
    ~RedSquare() override = default;

    bool init() override;
    void update(float dt) override;

    // Size used for collider and visuals (square).
    static inline constexpr float kSize = 24.f;

  private:
    void render(sf::RenderTarget& target) const override;

    // Visual
    sf::RectangleShape m_shape;
    sf::Vector2f       m_startCenter{0.f, 0.f};

    // Rotation
    static inline constexpr float kRotateDps = 90.f; // degrees per second
};
