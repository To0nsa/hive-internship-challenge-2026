#include "Player.h"
#include "../../../ResourceManager.h"
#include "../../../animation/Animation.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <cmath>

namespace {
// Animation clip names
constexpr const char* kIdle = "player_idle";
constexpr const char* kRun  = "player_run";
} // namespace

bool Player::init() {
    // Load animation textures if present
    const sf::Texture* idleTex = ResourceManager::getOrLoadTexture("PlayerIdleAnimation.png");
    const sf::Texture* runTex  = ResourceManager::getOrLoadTexture("PlayerMoveAnimation.png");

    // Create sprite from first available texture, or fallback
    if (!m_pSprite) {
        m_pSprite = std::make_unique<sf::Sprite>(*idleTex);
    }

    // Center origin for consistent transforms
    if (m_pSprite) {
        m_pSprite->setOrigin({kFrameSize.x * 0.5f, kFrameSize.y * 0.5f});
        m_spriteScale = kPlayerScale;
        applyFacingScale();
        m_pSprite->setPosition(m_position);
    }

    // Setup animator now that sprite exists
    if (m_pSprite)
        m_pAnimator = std::make_unique<SpriteAnimator>(*m_pSprite);

    // Build idle clip from a single row of square frames
    if (idleTex && m_pAnimator) {
        auto idleClip = Animation::makeClipFromRow(kIdle, *idleTex, kFrameSize, 4, 6.f, true);
        m_pAnimator->addClip(std::move(idleClip));
    }

    // Build run clip
    if (runTex && m_pAnimator) {
        auto runClip = Animation::makeClipFromRow(kRun, *runTex, kFrameSize, 7, 12.f, true);
        m_pAnimator->addClip(std::move(runClip));
    }

    // Enter initial state
    enterMove();
    return true;
}

void Player::enterMove() {
    m_state = State::Move;
    if (m_pAnimator) {
        if (!m_pAnimator->isPlayingClip(kIdle) && !m_pAnimator->isPlayingClip(kRun))
            m_pAnimator->playClip(kIdle);
    }
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

    // Integrate X only, Y handled in physics
    m_position.x += m_velocity.x * dt;
    if (m_pSprite)
        m_pSprite->setPosition(m_position);
}

void Player::update(float dt) {
    sf::Vector2f dir{0.f, 0.f};
    m_input.m_movingLeft  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
                            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
    m_input.m_movingRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
                            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
    if (m_input.m_movingLeft) dir.x = -1.f;
    else if (m_input.m_movingRight) dir.x = +1.f;

    if (m_state == State::Move) {
        // Update facing based on input
        if (m_input.m_movingLeft) setFacing(Entity::Facing::Left);
        else if (m_input.m_movingRight) setFacing(Entity::Facing::Right);
        applyMovement(dir, dt);
    }

    // Animation switching based on speed
    const float speedX = std::abs(m_velocity.x);
    if (m_pAnimator) {
        if (speedX > kMinSpeed)
            m_pAnimator->ensureClip(kRun);
        else
            m_pAnimator->ensureClip(kIdle);
        m_pAnimator->update(dt);
    }
}
