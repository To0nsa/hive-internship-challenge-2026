#pragma once

#include "../Actor.h"

struct PlayerInput {
    bool moveLeft    = false;
    bool moveRight   = false;
    bool jumpPressed = false; // edge-triggered
    bool dashPressed = false; // edge-triggered
    bool castPressed = false; // edge-triggered
};

class Player final : public Actor {
  public:
    Player()           = default;
    ~Player() override = default;

    bool init() override;
    void update(float dt) override;
    void setInput(const PlayerInput& input) { m_input = input; }

  private:
    // State machine
    enum class State { Move, Dash, Death, Cast };
    State m_state = State::Move;

    bool isGrounded() const;

    void enterMove();
    void enterDash(float dirX);
    void enterDeath();
    void enterCast();

    void applyMovement(const sf::Vector2f& direction, float dt);
    void tryApplyJump();
    void tryApplyDash();
    void tryApplyCast();

    void updateJumpAnimation();
    void updateMoveAnimation();

    // Input state
    PlayerInput m_input;
    bool m_jumpRequested = false;

    // Rendering
    static inline const sf::Vector2i kFrameSize{100, 64};
    static inline const sf::Vector2f kPlayerScale{1.5f, 1.5f};

    // Configuration
    static constexpr float kMaxSpeed     = 500.f;
    static constexpr float kAcceleration = 1200.f;
    static constexpr float kDeceleration = 800.f;
    static constexpr float kMinSpeed     = 5.f;
    static constexpr float kGravity      = 2400.f;

    // Stats
    static constexpr float kPlayerHpMax            = 100.f;
    static constexpr float kPlayerHpRegenRate      = 1.f;
    static constexpr float kPlayerManaMax          = 100.f;
    static constexpr float kPlayerManaRegenRate    = 3.f;
    static constexpr float kPlayerStaminaMax       = 100.f;
    static constexpr float kPlayerStaminaRegenRate = 1.f;
    static constexpr float kJumpStaminaCost        = 2.f;
    static constexpr float kDashStaminaCost        = 2.f;

    // Jumping
    static constexpr float kJumpSpeed       = 1200.f;
    static constexpr float kCoyoteTime      = 0.10f;
    static constexpr float kJumpBufferTime  = 0.12f;
    float                         m_coyoteTimer    = 0.f;
    float                         m_jumpBufferLeft = 0.f;

    // Dash
    static constexpr float kDashSpeed         = 1100.f;
    static constexpr float kDashDuration      = 0.20f;
    static constexpr float kDashCooldown      = 3.f;
    bool                          m_dashRequested    = false;
    float                         m_dashTimer        = 0.f;
    float                         m_dashCooldownLeft = 0.f;
    float                         m_dashDirX         = 0.f; // -1 left, +1 right
};
