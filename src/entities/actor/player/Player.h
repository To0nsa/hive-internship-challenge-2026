#pragma once

#include "../Actor.h"

#include <SFML/System/Angle.hpp>
#include <SFML/Window/Keyboard.hpp>

class Player final : public Actor {
  public:
    Player()           = default;
    ~Player() override = default;

    bool init() override;
    void update(float dt) override;

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
    struct Input {
        bool m_movingLeft  = false;
        bool m_movingRight = false;
    } m_input;
    bool m_prevJumpDown  = false;
    bool m_prevDashDown  = false;
    bool m_prevCastDown  = false;
    bool m_jumpRequested = false;

    // Rendering
    static inline const sf::Vector2i kFrameSize{100, 64};
    static inline const sf::Vector2f kPlayerScale{1.5f, 1.5f};

    // Configuration
    static inline constexpr float kMaxSpeed     = 500.f;
    static inline constexpr float kAcceleration = 1200.f;
    static inline constexpr float kDeceleration = 800.f;
    static inline constexpr float kMinSpeed     = 5.f;
    static inline constexpr float kGravity      = 2400.f;

    // Stats
    static inline constexpr float kPlayerHpMax            = 100.f;
    static inline constexpr float kPlayerHpRegenRate      = 1.f;
    static inline constexpr float kPlayerManaMax          = 100.f;
    static inline constexpr float kPlayerManaRegenRate    = 3.f;
    static inline constexpr float kPlayerStaminaMax       = 100.f;
    static inline constexpr float kPlayerStaminaRegenRate = 1.f;
    static inline constexpr float kJumpStaminaCost        = 2.f;
    static inline constexpr float kDashStaminaCost        = 2.f;

    // Jumping
    static inline constexpr float kJumpSpeed       = 1200.f;
    static inline constexpr float kCoyoteTime      = 0.10f;
    static inline constexpr float kJumpBufferTime  = 0.12f;
    float                         m_coyoteTimer    = 0.f;
    float                         m_jumpBufferLeft = 0.f;

    // Dash
    static inline constexpr float kDashSpeed         = 1400.f;
    static inline constexpr float kDashDuration      = 0.20f;
    static inline constexpr float kDashCooldown      = 3.f;
    bool                          m_dashRequested    = false;
    float                         m_dashTimer        = 0.f;
    float                         m_dashCooldownLeft = 0.f;
    float                         m_dashDirX         = 0.f; // -1 left, +1 right
};
