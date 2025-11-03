// src/game/Actor.cpp
#include "Actor.h"

#include "../../Config.h"
#include "../../Debug.h"
#include "../../ResourceManager.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <algorithm>
#include <cmath>

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

void Actor::updateActorBase(float dt) {
    regenerateHp(dt);
    regenerateMana(dt);
    regenerateStamina(dt);
}
