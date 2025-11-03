#pragma once

#include <memory>
#include <cmath>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Sprite.hpp>
// Animator is referenced via unique_ptr to avoid hard coupling
#include "../animation/Animation.h"

class StatePlaying; // forward declaration for world back-reference

namespace sf { class RenderTarget; };

class Entity
{
public:
    Entity() = default;
    virtual ~Entity() = default;

    virtual bool init() = 0;
    virtual void update(float dt) = 0;
    virtual void render(sf::RenderTarget& target) const = 0;

    bool isAlive() const { return m_alive; }
    void setAlive(bool alive) { m_alive = alive; }

    const sf::Vector2f& getPosition() const { return m_position; }
    void setPosition(const sf::Vector2f& position) { m_position = position; };

    // Optional back-reference to owning world (StatePlaying)
    void setWorld(StatePlaying* world) { m_world = world; }
    StatePlaying* getWorld() const { return m_world; }

protected:
    // State
    bool m_alive = true;

    // Render
    std::unique_ptr<sf::Sprite> m_pSprite;
    sf::Vector2f m_spriteScale{1.f, 1.f};
    // Rendering transform
    sf::Vector2f m_position;

    // Animation
    std::unique_ptr<SpriteAnimator> m_pAnimator;

    // Facing
    enum class Facing { Left, Right };
    Facing m_facing = Facing::Right;
    float  m_artFacingDirX = +1.f; // +1 = art facing right, -1 = art facing left

    void setFacing(Facing facing)
    {
        m_facing = facing;
        applyFacingScale();
    }
    void setArtFacingDirX(float directionX)
    {
        m_artFacingDirX = (directionX >= 0.f) ? +1.f : -1.f;
        applyFacingScale();
    }
    void applyFacingScale()
    {
        if (!m_pSprite)
            return;
        const float dir = (m_facing == Facing::Right) ? +1.f : -1.f;
        m_pSprite->setScale({dir * std::abs(m_spriteScale.x) * m_artFacingDirX, m_spriteScale.y});
    }

    // Physics and movement
    sf::Vector2f m_velocity;

public:
    // State playing is my world <3
    StatePlaying* m_world = nullptr;
};
