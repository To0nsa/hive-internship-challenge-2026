#pragma once

#include "entities/Entity.h"

#include <SFML/System/Vector2.hpp>

class Collider;

struct DamageInfo {
    float amount = 0.f;
    sf::Vector2f hitDirection; // Normalized direction vector of the hit
    sf::Vector2f hitPoint;     // World-space point where the hit occurred
};

class Actor : public Entity {
  public:
    Actor()          = default;
    virtual ~Actor() = default;

    // Stats
    void setInitialStats(float hpMax, float manaMax, float manaRegenRate, float hpRegenRate,
                         float staminaMax, float staminaRegenRate);

    void regenerateMana(float dt);
    void regenerateHp(float dt);
    void regenerateStamina(float dt);

    // Getters
    float getHp() const;
    float getHpMax() const;
    float getMana() const;
    float getManaMax() const;
    float getStamina() const;
    float getStaminaMax() const;

    void applyDamage(float dmg);

    // Minimal vertical physics with top-only collision against provided ground collider.
    // Pass a MultiRectCollider (preferred) or any Collider containing the walkable surfaces.
    virtual void applyPhysics(float dt, const Collider* ground);

  protected:
    virtual void updateActorBase(float dt);

    // Stats
    // Health
    float m_hp          = 1.f;
    float m_hpMax       = 1.f;
    float m_hpRegenRate = 0.f;
    // Mana
    float m_mana          = 0.f;
    float m_manaMax       = 0.f;
    float m_manaRegenRate = 0.f;
    // Stamina
    float m_stamina          = 0.f;
    float m_staminaMax       = 0.f;
    float m_staminaRegenRate = 0.f;

    // Rendering
    void render(sf::RenderTarget& target) const override;

    // Grounded state updated by applyPhysics
    bool m_grounded = false;
};
