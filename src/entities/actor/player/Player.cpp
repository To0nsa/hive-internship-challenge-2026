#include "entities/actor/player/Player.h"

#include "animation/Animation.h"
#include "collision/CollisionLayers.h"
#include "collision/RectCollider.h"
#include "core/Assets.h"
#include "core/ResourceManager.h"
#include "core/World.h"
#include "gameplay/Faction.h"
#include "physics/PhysicsBody.h"
#include "physics/StaticWorldGeometry.h"
#include "spell/CastRequest.h"
#include "spell/SpellCatalog.h"
#include "spell/projectile/Projectile.h"
#include "utils/Geom.h"

#include <SFML/Graphics/Sprite.hpp>
#include <cmath>

bool Player::init() {
    // Setup collision layer and mask
    setCollisionLayer(CollisionLayer::Player);
    setCollisionMask(maskFrom({CollisionLayer::Obstacle, CollisionLayer::Platform,
                               CollisionLayer::Collectible, CollisionLayer::EnemyProjectile}));

    // Load animation textures if present
    const sf::Texture& idleTex  = ResourceManager::getTexture(Assets::Tex::Player::Idle);
    const sf::Texture& runTex   = ResourceManager::getTexture(Assets::Tex::Player::Move);
    const sf::Texture& jumpTex  = ResourceManager::getTexture(Assets::Tex::Player::Jump);
    const sf::Texture& fallTex  = ResourceManager::getTexture(Assets::Tex::Player::Fall);
    const sf::Texture& dashTex  = ResourceManager::getTexture(Assets::Tex::Player::Dash);
    const sf::Texture& deathTex = ResourceManager::getTexture(Assets::Tex::Player::Death);
    const sf::Texture& castTex  = ResourceManager::getTexture(Assets::Tex::Player::Cast);
    const sf::Texture& hitTex   = ResourceManager::getTexture(Assets::Tex::Player::Hit);

    // Setup sprite
    m_pSprite = std::make_unique<sf::Sprite>(idleTex);
    m_pSprite->setOrigin({kFrameSize.x * 0.5f, kFrameSize.y * 0.5f});
    m_spriteScale = kPlayerScale;
    applyFacingScale();
    m_pSprite->setPosition(m_position);

    // Setup animator
    m_pAnimator = std::make_unique<SpriteAnimator>(*m_pSprite);

    // Build animation clips
    m_idleClip =
        m_pAnimator->addClip(Animation::makeClipFromRow("idle", idleTex, kFrameSize, 4, 6.f, true));
    m_runClip =
        m_pAnimator->addClip(Animation::makeClipFromRow("run", runTex, kFrameSize, 7, 12.f, true));
    m_jumpClip = m_pAnimator->addClip(
        Animation::makeClipFromRow("jump", jumpTex, kFrameSize, 6, 12.f, false));
    m_fallClip = m_pAnimator->addClip(
        Animation::makeClipFromRow("fall", fallTex, kFrameSize, 3, 12.f, true));
    m_dashClip = m_pAnimator->addClip(
        Animation::makeClipFromRow("dash", dashTex, kFrameSize, 4, 20.f, false));
    m_deathClip = m_pAnimator->addClip(
        Animation::makeClipFromRow("death", deathTex, kFrameSize, 6, 8.f, false));
    m_castClip = m_pAnimator->addClip(
        Animation::makeClipFromRow("cast", castTex, kFrameSize, 5, 20.f, false));
    m_hitClip =
        m_pAnimator->addClip(Animation::makeClipFromRow("hit", hitTex, kFrameSize, 4, 10.f, false));

    // Setup collider
    setColliderSize(
        {static_cast<float>(kFrameSize.x) * 0.2f, static_cast<float>(kFrameSize.y) * 0.88f});

    // Register with physics system so movement and gravity are applied centrally.
    PhysicsBodyConfig physCfg;
    physCfg.enabled       = true;
    physCfg.isKinematic   = false;
    physCfg.useGravity    = true;
    physCfg.topOnlyGround = true;
    physCfg.gravityScale  = 1.f;
    physCfg.maxVelX       = kMaxVelX;
    physCfg.maxVelY       = kMaxVelY;
    physCfg.sideMask      = static_cast<std::uint8_t>(StaticSolidSide::SolidSide_Left) |
                       static_cast<std::uint8_t>(StaticSolidSide::SolidSide_Right);
    m_world->getPhysics().registerBody(*this, physCfg);

    // Setup stats
    setInitialStats(kPlayerHpMax, kPlayerManaMax, kPlayerManaRegenRate, kPlayerHpRegenRate,
                    kPlayerStaminaMax, kPlayerStaminaRegenRate);

    // Enter initial state
    enterMove();
    return true;
}

void Player::applyHorizontalMovement(const sf::Vector2f& direction, float dt) {
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
}

void Player::update(float dt) {
    // Death state: if HP or Stamina is zero or below, enter death and only update animation.
    if ((m_hp <= 0.f || m_stamina <= 0.f) && m_state != State::Death) {
        enterDeath();
    }

    // Death state: only update death animation.
    if (m_state == State::Death) {
        m_pAnimator->playClip(m_deathClip);
        m_pAnimator->update(dt);
        m_pSprite->setPosition(m_position);
        return;
    }

    // Base actor update (regeneration, etc)
    updateActorBase(dt);

    // Consume input supplied by the state
    if (m_input.jumpPressed) {
        // Start / refresh jump buffer window on edge-triggered press
        m_jumpBufferLeft = kJumpBufferTime;
    }
    if (m_input.dashPressed) {
        m_dashRequested = true;
    }

    // Timers: coyote time and jump buffer
    if (isGrounded()) {
        m_coyoteTimer = kCoyoteTime;
    } else {
        m_coyoteTimer -= dt;
    }

    // Jump buffer timer
    if (m_jumpBufferLeft > 0.f) {
        m_jumpBufferLeft -= dt;
        if (m_jumpBufferLeft < 0.f)
            m_jumpBufferLeft = 0.f;
    }

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
    if (m_input.moveLeft)
        dir.x -= 1.f;
    if (m_input.moveRight)
        dir.x += 1.f;
    if (dir.x != 0.f)
        dir.x = (dir.x > 0.f) ? 1.f : -1.f;

    // Facing updates only during normal movement
    if (m_state == State::Move) {
        if (m_input.moveLeft)
            setFacing(Entity::Facing::Left);
        else if (m_input.moveRight)
            setFacing(Entity::Facing::Right);
    }

    // Try dash when not already dashing
    if (m_state != State::Dash && m_state != State::Cast)
        tryApplyDash();

    // Try cast
    if (m_input.castPressed && m_state != State::Dash && m_state != State::Death)
        enterCast();

    // Apply horizontal movement
    if (m_state == State::Move) {
        applyHorizontalMovement(dir, dt);
    } else if (m_state == State::Dash) {
        m_velocity.x = m_dashDirX * kDashSpeed;
    } else if (m_state == State::Cast) {
        // Allow light movement during cast
        applyHorizontalMovement(dir, dt);
    }

    // Jumping (disabled while dashing)
    if (m_state != State::Dash)
        tryApplyJump();

    // Animation selection
    if (m_state == State::Dash) {
        m_pAnimator->playClip(m_dashClip);
    } else if (m_state == State::Cast) {
        m_pAnimator->playClip(m_castClip);
    } else if (m_state == State::Hit) {
        m_pAnimator->playClip(m_hitClip);
    } else if (!isGrounded()) {
        updateJumpAnimation();
    } else {
        updateMoveAnimation();
    }
    m_pAnimator->update(dt);

    // Sprite position follows entity transform (via Entity::setPosition / PhysicsSystem).
}

void Player::tryApplyJump() {
    // Already grounded => nothing to do
    if (!(isGrounded() || m_coyoteTimer > 0.f))
        return;

    // No buffered jump request => nothing to do
    if (m_jumpBufferLeft <= 0.f)
        return;

    if (m_stamina < kJumpStaminaCost)
        return;

    // Perform the jump
    m_velocity.y = -kJumpForce;
    m_stamina -= kJumpStaminaCost;

    // Consume the buffer so we don't chain extra jumps
    m_jumpBufferLeft = 0.f;

    m_pAnimator->requestRestart();
}

void Player::tryApplyDash() {
    if (!m_dashRequested)
        return;
    m_dashRequested = false;
    if (m_dashCooldownLeft > 0.f)
        return;
    if (m_stamina < kDashStaminaCost)
        return;

    float dirX = 0.f;
    if (m_input.moveLeft)
        dirX = -1.f;
    else if (m_input.moveRight)
        dirX = +1.f;
    else
        dirX = (m_facing == Facing::Right) ? +1.f : -1.f;
    enterDash(dirX);
}

void Player::updateJumpAnimation() {
    if (m_velocity.y < 0.f) {
        m_pAnimator->playClip(m_jumpClip);
    } else {
        m_pAnimator->playClip(m_fallClip);
    }
}

void Player::updateMoveAnimation() {
    const float speed = std::abs(m_velocity.x);
    if (speed > kMinSpeed)
        m_pAnimator->playClip(m_runClip);
    else
        m_pAnimator->playClip(m_idleClip);
}

void Player::enterMove() {
    m_state = State::Move;
    m_pAnimator->playClip(m_idleClip);

    // Restore normal gravity after dash or other special states.
    if (auto* body = m_world->getPhysics().findBody(*this)) {
        body->config.useGravity = true;
    }
}

void Player::enterDash(float dirX) {
    m_state            = State::Dash;
    m_dashTimer        = kDashDuration;
    m_dashCooldownLeft = kDashCooldown;
    m_dashDirX         = (dirX >= 0.f) ? +1.f : -1.f;
    // Reset vertical velocity so dash does not inherit any upward or downward motion,
    // and temporarily disable gravity while dashing.
    m_velocity.y = 0.f;
    if (auto* body = m_world->getPhysics().findBody(*this)) {
        body->config.useGravity = false;
    }
    if (m_stamina >= kDashStaminaCost)
        m_stamina -= kDashStaminaCost;
    setFacing(m_dashDirX > 0 ? Facing::Right : Facing::Left);
    m_pAnimator->playClip(m_dashClip);
}

void Player::enterDeath() {
    m_state = State::Death;

    m_pAnimator->playClip(m_deathClip, [world = m_world]() { world->requestExitToMenu(); });
}

void Player::enterCast() {
    // Spend mana and spawn a projectile if possible
    const SpellDef& def = getSpellDef(SpellId::IceBolt);
    if (m_mana < def.stats.manaCost)
        return;
    m_mana -= def.stats.manaCost;

    // Aim: from player position to mouse world position
    sf::Vector2f       aimDir{(m_facing == Facing::Right) ? +1.f : -1.f, 0.f};
    const sf::Vector2f mouseWorld = m_world->getMouseWorld();
    sf::Vector2f       v          = mouseWorld - m_position;
    sf::Vector2f       n          = math::normalizeVec(v);
    if (n.x != 0.f || n.y != 0.f)
        aimDir = n;

    // Origin slightly in front along aim
    sf::Vector2f orig = m_position + aimDir * (kFrameSize.x * 0.4f);

    auto* proj = m_world->createEntity<Projectile>(SpellId::IceBolt, Faction::Player, orig, aimDir);

    (void)proj->init();

    // Play cast animation and return to move on completion
    m_state = State::Cast;
    m_pAnimator->playClip(m_castClip, [this]() {
        if (m_state == State::Cast)
            enterMove();
    });
}

void Player::enterHit() {
    m_state = State::Hit;

    m_pAnimator->playClip(m_hitClip, [this]() {
        if (m_state == State::Hit)
            enterMove();
    });
}

bool Player::isGrounded() const {
    const PhysicsBody* body = m_world->getPhysics().findBody(*this);
    return body->grounded;
}

void Player::onDamaged(const DamageInfo& damage) {
    if (m_state == State::Death)
        return;

    if (m_state == State::Hit)
        return;

    if (damage.collideWith == CollisionLayer::EnemyProjectile)
        enterHit();
}
