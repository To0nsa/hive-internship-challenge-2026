#include "Player.h"

#include "../../../ResourceManager.h"
#include "../../../animation/Animation.h"
#include "../../../collision/RectCollider.h"
#include "../../../gamestates/StatePlaying.h"
#include "../../../utils/Geom.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Window/Mouse.hpp>
#include <cmath>

namespace {
    // Animation clip names
    constexpr const char* kIdle = "player_idle";
    constexpr const char* kRun  = "player_run";
    constexpr const char* kJump = "player_jump";
    constexpr const char* kFall = "player_fall";
    constexpr const char* kDash = "player_dash";
    constexpr const char* kDeath = "player_death";
} // namespace

bool Player::init() {
    // Load animation textures if present
    const sf::Texture* idleTex = ResourceManager::getOrLoadTexture("PlayerIdleAnimation.png");
    const sf::Texture* runTex  = ResourceManager::getOrLoadTexture("PlayerMoveAnimation.png");
    const sf::Texture* jumpTex = ResourceManager::getOrLoadTexture("PlayerJumpAnimation.png");
    const sf::Texture* fallTex = ResourceManager::getOrLoadTexture("PlayerFallAnimation.png");
    const sf::Texture* dashTex = ResourceManager::getOrLoadTexture("PlayerDashAnimation.png");
    const sf::Texture* deathTex = ResourceManager::getOrLoadTexture("PlayerDeathAnimation.png");

    // Setup sprite
    m_pSprite = std::make_unique<sf::Sprite>(*idleTex);
    m_pSprite->setOrigin({kFrameSize.x * 0.5f, kFrameSize.y * 0.5f});
    m_spriteScale = kPlayerScale;
    applyFacingScale();
    m_pSprite->setPosition(m_position);

    // Setup animator
    m_pAnimator = std::make_unique<SpriteAnimator>(*m_pSprite);

    // Build animation clips
    auto idleClip = Animation::makeClipFromRow(kIdle, *idleTex, kFrameSize, 4, 6.f, true);
    m_pAnimator->addClip(std::move(idleClip));
    auto runClip = Animation::makeClipFromRow(kRun, *runTex, kFrameSize, 7, 12.f, true);
    m_pAnimator->addClip(std::move(runClip));
    auto jumpClip = Animation::makeClipFromRow(kJump, *jumpTex, kFrameSize, 6, 12.f, false);
    m_pAnimator->addClip(std::move(jumpClip));
    auto fallClip = Animation::makeClipFromRow(kFall, *fallTex, kFrameSize, 3, 12.f, true);
    m_pAnimator->addClip(std::move(fallClip));
    auto dashClip = Animation::makeClipFromRow(kDash, *dashTex, kFrameSize, 4, 20.f, false);
    m_pAnimator->addClip(std::move(dashClip));
    auto deathClip = Animation::makeClipFromRow(kDeath, *deathTex, kFrameSize, 6, 8.f, false);
    m_pAnimator->addClip(std::move(deathClip));

    // Setup collider
    setColliderSize(
        {static_cast<float>(kFrameSize.x) * 0.2f, static_cast<float>(kFrameSize.y) * 0.88f});

    // Setup stats
    setInitialStats(kPlayerHpMax, kPlayerManaMax, kPlayerManaRegenRate, kPlayerHpRegenRate,
                    kPlayerStaminaMax, kPlayerStaminaRegenRate);

    // Enter initial state
    enterMove();
    return true;
}

void Player::applyMovement(const sf::Vector2f& direction, float dt) {
    // Horizontal move
    if (direction.x != 0.f) {
        // Accelerate toward desired X
        const float desiredX = direction.x * kMaxSpeed;
        const float deltaX   = desiredX - m_velocity.x;
        const float maxDelta = kAcceleration * dt;
        if (std::abs(deltaX) > maxDelta) {
            m_velocity.x += (deltaX > 0.f ? +maxDelta : -maxDelta);
        } else {
            m_velocity.x = desiredX;
        }
    } else {
        // Decelerate X toward 0
        const float speedX = std::abs(m_velocity.x);
        if (speedX > 0.f) {
            const float drop = kDeceleration * dt;
            if (speedX <= drop || speedX <= kMinSpeed) {
                m_velocity.x = 0.f;
            } else {
                m_velocity.x += (m_velocity.x > 0.f ? -drop : +drop);
            }
        }
    }

    // Integrate X only; Y handled separately
    m_position.x += m_velocity.x * dt;
}

void Player::update(float dt) {
    // Base actor update (regen, etc.)
    updateActorBase(dt);

    // Edge-triggered input for jump/dash
    const bool leftDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
                          sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
    const bool rightDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
                           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
    const bool jumpDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
    const bool dashDown = sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle);

    const bool jumpPressed = jumpDown && !m_prevJumpDown;
    const bool dashPressed = dashDown && !m_prevDashDown;
    m_prevJumpDown         = jumpDown;
    m_prevDashDown         = dashDown;

    // Snapshot input
    m_input.m_movingLeft  = leftDown;
    m_input.m_movingRight = rightDown;
    if (jumpPressed) {
        m_jumpRequested  = true;
        m_jumpBufferLeft = kJumpBufferTime;
    }
    if (dashPressed) {
        m_dashRequested = true;
    }

    // Timers: coyote time and jump buffer
    if (isGrounded()) {
        m_coyoteTimer = kCoyoteTime;
    } else {
        m_coyoteTimer -= dt;
    }
    if (m_jumpBufferLeft > 0.f)
        m_jumpBufferLeft -= dt;

    // Dash timers and transitions
    if (m_dashCooldownLeft > 0.f)
        m_dashCooldownLeft -= dt;
    if (m_dashCooldownLeft < 0.f)
        m_dashCooldownLeft = 0.f;
    if (m_state == State::Dash) {
        m_dashTimer -= dt;
        if (m_dashTimer <= 0.f)
            enterMove();
    }

    // Horizontal direction
    sf::Vector2f dir{0.f, 0.f};
    if (m_input.m_movingLeft)
        dir.x -= 1.f;
    if (m_input.m_movingRight)
        dir.x += 1.f;
    if (dir.x != 0.f)
        dir.x = (dir.x > 0.f) ? 1.f : -1.f;

    // Facing updates only during normal movement
    if (m_state == State::Move) {
        if (m_input.m_movingLeft)
            setFacing(Entity::Facing::Left);
        else if (m_input.m_movingRight)
            setFacing(Entity::Facing::Right);
    }

    // Try dash when not already dashing
    if (m_state != State::Dash)
        tryApplyDash();

    // Apply horizontal movement
    if (m_state == State::Move) {
        applyMovement(dir, dt);
    } else if (m_state == State::Dash) {
        m_velocity.x = m_dashDirX * kDashSpeed;
        m_position.x += m_velocity.x * dt;
    }

    // Jumping (disabled while dashing)
    if (m_state != State::Dash)
        tryApplyJump();

    // Vertical physics and collision are applied from the world via Actor::applyPhysics().

    // Animation selection
    if (m_pAnimator) {
        if (m_state == State::Dash) {
            m_pAnimator->ensureClip(kDash);
        } else if (!isGrounded()) {
            updateJumpAnimation();
        } else {
            updateMoveAnimation();
        }
        m_pAnimator->update(dt);
    }

    // Push transform to sprite
    if (m_pSprite)
        m_pSprite->setPosition(m_position);
}

void Player::tryApplyJump() {
    if ((m_jumpRequested || m_jumpBufferLeft > 0.f) && (isGrounded() || m_coyoteTimer > 0.f)) {
        m_velocity.y     = -kJumpSpeed;
        m_jumpRequested  = false;
        m_jumpBufferLeft = 0.f;
        if (m_pAnimator)
            m_pAnimator->requestRestart();
    }
}

void Player::tryApplyDash() {
    if (!m_dashRequested)
        return;
    m_dashRequested = false;
    if (m_dashCooldownLeft > 0.f)
        return;

    float dirX = 0.f;
    if (m_input.m_movingLeft)
        dirX = -1.f;
    else if (m_input.m_movingRight)
        dirX = +1.f;
    else
        dirX = (m_facing == Facing::Right) ? +1.f : -1.f;
    enterDash(dirX);
}

void Player::updateJumpAnimation() {
    if (m_velocity.y < 0.f) {
        m_pAnimator->ensureClip(kJump);
    } else {
        m_pAnimator->ensureClip(kFall);
    }
}

void Player::updateMoveAnimation() {
    const float speed = std::abs(m_velocity.x);
    if (speed > kMinSpeed)
        m_pAnimator->ensureClip(kRun);
    else
        m_pAnimator->ensureClip(kIdle);
}

void Player::enterMove() {
    m_state = State::Move;
    if (m_pAnimator) {
        if (!m_pAnimator->isPlayingClip(kIdle) && !m_pAnimator->isPlayingClip(kRun))
            m_pAnimator->playClip(kIdle);
    }
}

void Player::enterDash(float dirX) {
    m_state            = State::Dash;
    m_dashTimer        = kDashDuration;
    m_dashCooldownLeft = kDashCooldown;
    m_dashDirX         = (dirX >= 0.f) ? +1.f : -1.f;
    setFacing(m_dashDirX > 0 ? Facing::Right : Facing::Left);
    if (m_pAnimator)
        m_pAnimator->playClip(kDash);
}

void Player::enterDeath() {
    m_state = State::Death;
    if (m_pAnimator)
        m_pAnimator->playClip(kDeath);
}

bool Player::isGrounded() const { return m_grounded; }
