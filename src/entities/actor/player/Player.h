#pragma once

#include "../Actor.h"
#include <SFML/System/Angle.hpp>
#include <SFML/Window/Keyboard.hpp>

class Player final : public Actor {
  public:

    Player() = default;
    ~Player() override = default;

    bool init() override;
    void update(float dt) override;

    // State control
    void enterMove();

  private:
    enum class State { Move };
    State m_state = State::Move;

    void applyMovement(const sf::Vector2f& direction, float dt);

    // Input state
    struct Input {
        bool m_movingLeft  = false;
        bool m_movingRight = false;
    } m_input;

    // Rendering
    static inline const sf::Vector2i kFrameSize{100, 64};
    static inline const sf::Vector2f kPlayerScale{1.5f, 1.5f};

    // Configuration
    static inline constexpr float kMaxSpeed     = 500.f;
    static inline constexpr float kAcceleration = 1200.f;
    static inline constexpr float kDeceleration = 600.f;
    static inline constexpr float kMinSpeed     = 5.f;

    // Stats
    static inline constexpr float kPlayerHpMax            = 100.f;
    static inline constexpr float kPlayerHpRegenRate      = 1.f;
    static inline constexpr float kPlayerManaMax          = 100.f;
    static inline constexpr float kPlayerManaRegenRate    = 1.f;
    static inline constexpr float kPlayerStaminaMax       = 100.f;
    static inline constexpr float kPlayerStaminaRegenRate = 1.f;
};
