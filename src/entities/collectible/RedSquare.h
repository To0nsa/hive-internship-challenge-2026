#pragma once

#include "collision/CollisionLayers.h"
#include "entities/Entity.h"

#include <SFML/Graphics/RectangleShape.hpp>

// Minimal rotating collectible square.
class RedSquare final : public Entity {
  public:
    explicit RedSquare(const sf::Vector2f& center) : m_startCenter(center) {
        setCollisionLayer(CollisionLayer::Collectible);
        setCollisionMask(maskFrom({CollisionLayer::Player, CollisionLayer::PlayerProjectile}));
    }
    ~RedSquare() override = default;

    bool init() override;
    void update(float dt) override;

    // Size used for collider and visuals (square).
    static constexpr float kSize = 24.f;

  private:
    void render(sf::RenderTarget& target) const override;

    // Visual
    sf::RectangleShape m_shape;
    sf::Vector2f       m_startCenter{0.f, 0.f};

    // Rotation
    static constexpr float kRotateDps = 90.f; // degrees per second
};
