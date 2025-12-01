#pragma once

#include "animation/Animation.h"
#include "entities/actor/enemy/Enemy.h"

#include <SFML/System/Vector2.hpp>

class FireWorm final : public Enemy {
  public:
    FireWorm()           = default;
    ~FireWorm() override = default;

    bool init() override;
    void update(float dt) override;

    void applyPhysics(float /*dt*/, const Collider* /*ground*/) override {}

  private:
    enum class State { Move, Idle, Attack, Death, Hit };
    State m_state = State::Idle;

    void enterMove();
    void enterIdle();
    void enterAttack();
    void enterHit();
    void enterDeath();

    void updateMove(float dt);

    // React to damage by briefly entering the hit animation state.
    void onDamaged(const DamageInfo& info) override;

    // Casting
    static constexpr float kCastCooldown      = 2.0f;
    float                  m_castCooldownLeft = 0.f;

    // Movement goal: hover around desired range from player
    static constexpr float kDesiredRange = 300.f;
    static constexpr float kHoldSlack    = 30.f;  // acceptable band around desired
    static constexpr float kMoveSpeed    = 800.f; // px/s

    // Stats
    static constexpr float kFireWormHpMax         = 50.f;
    static constexpr float kFireWormManaMax       = 80.f;
    static constexpr float kFireWormHpRegenRate   = 0.f;
    static constexpr float kFireWormManaRegenRate = 5.f;

    static inline const sf::Vector2i kFrameSize{90, 90};
    static inline const sf::Vector2f kSpriteScale{1.2f, 1.2f};
    static inline const sf::Vector2f kColliderSizeMult{0.30f, 0.30f};
    SpriteAnimator::ClipId           m_moveClip   = SpriteAnimator::kInvalidClip;
    SpriteAnimator::ClipId           m_idleClip   = SpriteAnimator::kInvalidClip;
    SpriteAnimator::ClipId           m_attackClip = SpriteAnimator::kInvalidClip;
    SpriteAnimator::ClipId           m_deathClip  = SpriteAnimator::kInvalidClip;
    SpriteAnimator::ClipId           m_hitClip    = SpriteAnimator::kInvalidClip;
};
