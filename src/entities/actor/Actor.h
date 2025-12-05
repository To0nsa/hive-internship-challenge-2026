#pragma once

#include "entities/Entity.h"
#include "gameplay/Damage.h"

#include <SFML/System/Vector2.hpp>

class Collider;

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

    // Main damage entry point using hit data.
    void applyDamage(const DamageInfo& info);

    // Minimal vertical physics with top-only collision against provided ground collider.
    // Pass a MultiRectCollider (preferred) or any Collider containing the walkable surfaces.
    virtual void applyPhysics(float dt, const Collider* ground);

  protected:
    virtual void updateActorBase(float dt);

    // Hook for derived classes to react to damage (animations, knockback, etc.).
    virtual void onDamaged(const DamageInfo& info);

    // Physics constants
    static constexpr float kPhysGravity = 2400.f;
    static constexpr float kMaxVelY     = 3000.f;

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
