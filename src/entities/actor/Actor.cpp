// src/game/Actor.cpp
#include "Actor.h"

#include "../../Config.h"
#include "../../Debug.h"
#include "../../ResourceManager.h"
#include "../../collision/MultiRectCollider.h"
#include "../../utils/Geom.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <algorithm>
#include <cmath>
#include <limits>

void Actor::render(sf::RenderTarget& target) const {
    if (m_pSprite)
        target.draw(*m_pSprite);
    if (m_pSprite && Config::kDebugDraw) {
        Debug::drawSpriteBounds(target, *m_pSprite, sf::Color::Red, 1.f);
        Debug::drawSpriteOrigin(target, *m_pSprite, sf::Color::Yellow, 10.f, 2.f);
        Debug::drawColliderBounds(target, getCollider(), sf::Color::Green, 1.f);
    }
}

void Actor::setInitialStats(float hpMax, float manaMax, float manaRegenRate, float hpRegenRate,
                            float staminaMax, float staminaRegenRate) {
    m_hpMax            = std::max(0.f, hpMax);
    m_hp               = m_hpMax;
    m_hpRegenRate      = std::max(0.f, hpRegenRate);
    m_manaMax          = std::max(0.f, manaMax);
    m_mana             = m_manaMax;
    m_manaRegenRate    = std::max(0.f, manaRegenRate);
    m_staminaMax       = std::max(0.f, staminaMax);
    m_stamina          = m_staminaMax;
    m_staminaRegenRate = std::max(0.f, staminaRegenRate);
}

void Actor::regenerateMana(float dt) {
    if (m_mana < m_manaMax) {
        m_mana += m_manaRegenRate * dt;
        if (m_mana > m_manaMax)
            m_mana = m_manaMax;
    }
}

void Actor::regenerateHp(float dt) {
    if (m_hp < m_hpMax) {
        m_hp += m_hpRegenRate * dt;
        if (m_hp > m_hpMax)
            m_hp = m_hpMax;
    }
}

void Actor::regenerateStamina(float dt) {
    if (m_stamina < m_staminaMax) {
        m_stamina += m_staminaRegenRate * dt;
        if (m_stamina > m_staminaMax)
            m_stamina = m_staminaMax;
    }
}

float Actor::getHp() const { return m_hp; }
float Actor::getHpMax() const { return m_hpMax; }
float Actor::getMana() const { return m_mana; }
float Actor::getManaMax() const { return m_manaMax; }
float Actor::getStamina() const { return m_stamina; }
float Actor::getStaminaMax() const { return m_staminaMax; }

void Actor::applyDamage(float dmg) {
    if (dmg <= 0.f)
        return;
    m_hp -= dmg;
    if (m_hp < 0.f)
        m_hp = 0.f;
}

void Actor::updateActorBase(float dt) {
    regenerateHp(dt);
    regenerateMana(dt);
    regenerateStamina(dt);
}

void Actor::applyPhysics(float dt, const Collider* ground) {
    // Tunables kept simple for prototype
    static constexpr float kPhysGravity = 2400.f;
    static constexpr float kMaxVelY     = 3000.f;

    // Gravity + vertical move
    if (!m_grounded)
        m_velocity.y += kPhysGravity * dt;
    if (m_velocity.y > kMaxVelY)
        m_velocity.y = kMaxVelY;
    if (m_velocity.y != 0.f) {
        m_position.y += m_velocity.y * dt;
        if (m_pSprite)
            m_pSprite->setPosition(m_position);
    }

    // Ground collision (top-only). Skip if moving upward.
    m_grounded = false;
    if (m_velocity.y < 0.f || !ground)
        return;

    const sf::FloatRect actorCollider = getCollider().worldAabb();
    const float         actorR        = geom::right(actorCollider);
    const float         actorB        = geom::bottom(actorCollider);

    float closestLiftDy = -std::numeric_limits<float>::infinity(); // largest deltaY ≤ 0

    auto consider = [&](const sf::FloatRect& groundCollider) {
        const float groundR = geom::right(groundCollider);
        // Only collide if horizontally overlapping
        if (actorR <= geom::left(groundCollider) || geom::left(actorCollider) >= groundR)
            return;
        // Move up to rest on top
        const float deltaY = geom::top(groundCollider) - actorB;
        if (deltaY <= 0.f)
            closestLiftDy = std::max(closestLiftDy, deltaY);
    };

    if (const auto* multi = dynamic_cast<const MultiRectCollider*>(ground)) {
        for (const auto& r : multi->getRectColliders())
            consider(r);
    } else {
        consider(ground->worldAabb());
    }

    if (closestLiftDy > -std::numeric_limits<float>::infinity()) {
        m_position.y += closestLiftDy;
        if (m_pSprite)
            m_pSprite->setPosition(m_position);
        m_velocity.y = 0.f;
        m_grounded   = true;
    }
}
