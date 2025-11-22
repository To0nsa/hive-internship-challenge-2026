#pragma once

#include "Enemy.h"

#include <SFML/System/Vector2.hpp>

class Demon final : public Enemy {
  public:
    Demon()           = default;
    ~Demon() override = default;

    bool init() override;
    void update(float dt) override;

    // Flying: ignore Actor's vertical physics/gravity
    void applyPhysics(float /*dt*/, const Collider* /*ground*/) override {}

  private:
    enum class State { Fly, Death };
    State m_state = State::Fly;

    void enterFly();
    void enterDeath();

    void updateFly(float dt);

    // Casting
    static constexpr float kCastCooldown      = 2.0f;
    float                  m_castCooldownLeft = 0.f;

    // Movement goal: hover around desired range from player
    static constexpr float kDesiredRange = 300.f;
    static constexpr float kHoldSlack    = 30.f;  // acceptable band around desired
    static constexpr float kMoveSpeed    = 800.f; // px/s

    // Stats
    static constexpr float kDemonHpMax         = 50.f;
    static constexpr float kDemonManaMax       = 80.f;
    static constexpr float kDemonHpRegenRate   = 0.f;
    static constexpr float kDemonManaRegenRate = 5.f;
};
