// src/game/Actor.cpp
#include "Actor.h"

#include "../../core/Debug.h"
#include "Config.h"
#include "Geom.h"
#include "MultiRectCollider.h"
#include "ResourceManager.h"

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
    static constexpr float kPhysGravity = 2400.f;
    static constexpr float kMaxVelY     = 3000.f;

    // Gravity + vertical move
    if (!m_grounded)
        m_velocity.y += kPhysGravity * dt;
    if (m_velocity.y > kMaxVelY)
        m_velocity.y = kMaxVelY;

    const float dy = m_velocity.y * dt;
    if (m_velocity.y != 0.f) {
        m_position.y += dy;
        if (m_pSprite)
            m_pSprite->setPosition(m_position);
    }

    // Ground collision (top-only). Skip if moving upward.
    m_grounded = false;
    if (m_velocity.y < 0.f || !ground)
        return;

    const sf::FloatRect actorCollider = getCollider().worldAabb();
    sf::FloatRect       prevActor     = actorCollider;
    prevActor.position.y -= dy; // position before applying vertical integration

    // Choose the smallest upward correction needed to rest on top.
    float bestLiftDy  = -std::numeric_limits<float>::infinity(); // largest deltaY ≤ 0
    bool  touchingTop = false; // exact top contact without overlap

    auto consider = [&](const sf::FloatRect& groundCollider) {
        // Require actual overlap this frame to resolve.
        sf::FloatRect inter;
        // Track exact top contact even without overlap (resting on top, no motion)
        if (m_velocity.y == 0.f && geom::touchTop(actorCollider, groundCollider, 0.75f))
            touchingTop = true;

        // We only care about landings from above this frame: bottom crossing the top plane.
        const float groundTop    = geom::top(groundCollider);
        const float prevBottom   = geom::bottom(prevActor);
        const float currentB     = geom::bottom(actorCollider);
        const float currentLeft  = geom::left(actorCollider);
        const float currentRight = geom::right(actorCollider);
        const float groundLeft   = geom::left(groundCollider);
        const float groundRight  = geom::right(groundCollider);

        // Horizontal overlap required to stand on top.
        if (currentRight <= groundLeft || currentLeft >= groundRight)
            return;

        // Must be above last frame and now at/through the top plane.
        constexpr float kTopEps = 0.001f;
        if (prevBottom > groundTop + kTopEps)
            return; // not a landing from above
        if (currentB < groundTop - kTopEps)
            return; // still above top -> no contact yet

        // Upward lift required to rest bottom on top of ground.
        const float deltaY = groundTop - currentB; // ≤ 0 when penetrating from above
        bestLiftDy         = std::max(bestLiftDy, deltaY);
    };

    if (const auto* multi = dynamic_cast<const MultiRectCollider*>(ground)) {
        for (const auto& r : multi->getRectColliders())
            consider(r);
    } else {
        consider(ground->worldAabb());
    }

    if (bestLiftDy > -std::numeric_limits<float>::infinity()) {
        m_position.y += bestLiftDy;
        if (m_pSprite)
            m_pSprite->setPosition(m_position);
        m_velocity.y = 0.f;
        m_grounded   = true;
    } else if (touchingTop) {
        // Maintain grounded state on exact top contact
        m_grounded = true;
    }
}
