#include "entities/actor/enemy/FireWorm.h"

#include "animation/Animation.h"
#include "core/Assets.h"
#include "core/ResourceManager.h"
#include "core/World.h"
#include "entities/actor/player/Player.h"
#include "gameplay/Faction.h"
#include "spell/SpellCatalog.h"
#include "spell/projectile/Projectile.h"

bool FireWorm::init() {
    // Load textures
    sf::Texture& moveTex   = ResourceManager::getTexture(Assets::Tex::Enemy::FireWorm::Move);
    sf::Texture& idleTex   = ResourceManager::getTexture(Assets::Tex::Enemy::FireWorm::Idle);
    sf::Texture& attackTex = ResourceManager::getTexture(Assets::Tex::Enemy::FireWorm::Attack);
    sf::Texture& deathTex  = ResourceManager::getTexture(Assets::Tex::Enemy::FireWorm::Death);
    sf::Texture& hitTex    = ResourceManager::getTexture(Assets::Tex::Enemy::FireWorm::Hit);

    // Sprite
    m_pSprite = std::make_unique<sf::Sprite>(moveTex);
    m_pSprite->setOrigin({kFrameSize.x * 0.5f, kFrameSize.y * 0.5f});
    m_spriteScale = {kSpriteScale.x, kSpriteScale.y};
    applyFacingScale();
    m_pSprite->setPosition(m_position);

    // Stats
    setInitialStats(kFireWormHpMax, kFireWormManaMax, kFireWormManaRegenRate, kFireWormHpRegenRate,
                    0.f, 0.f);

    // Animator
    m_pAnimator = std::make_unique<SpriteAnimator>(*m_pSprite);

    // Clips
    m_moveClip = m_pAnimator->addClip(
        Animation::makeClipFromRow("move", moveTex, kFrameSize, 9, 10.f, true));
    m_idleClip = m_pAnimator->addClip(
        Animation::makeClipFromRow("idle", idleTex, kFrameSize, 9, 10.f, true));
    m_attackClip = m_pAnimator->addClip(
        Animation::makeClipFromRow("attack", attackTex, kFrameSize, 16, 10.f, false));
    m_deathClip = m_pAnimator->addClip(
        Animation::makeClipFromRow("death", deathTex, kFrameSize, 8, 10.f, false));
    m_hitClip =
        m_pAnimator->addClip(Animation::makeClipFromRow("hit", hitTex, kFrameSize, 3, 10.f, false));
    // Collider
    setColliderSize({kFrameSize.x * kColliderSizeMult.x, kFrameSize.y * kColliderSizeMult.y});
    setArtFacingDirX(+1.f);
    enterMove();
    return true;
}

void FireWorm::onDamaged(const DamageInfo& /*info*/) {
    // Skip reactions once death has started so we don't interrupt the death animation.
    if (m_state == State::Death)
        return;

    // For now, any damaging hit just plays the hit animation; later we can add knockback, etc.
    if (m_state != State::Hit)
        enterHit();
}

void FireWorm::enterMove() {
    m_state = State::Move;
    m_pAnimator->playClip(m_moveClip);
}

void FireWorm::enterIdle() {
    m_state = State::Idle;
    m_pAnimator->playClip(m_idleClip);
}

void FireWorm::enterAttack() {
    m_state = State::Attack;
    m_pAnimator->playClip(m_attackClip, [this]() { enterMove(); });
}

void FireWorm::enterHit() {
    m_state = State::Hit;
    m_pAnimator->playClip(m_hitClip, [this]() { enterMove(); });
}

void FireWorm::enterDeath() {
    m_state = State::Death;
    m_pAnimator->playClip(m_deathClip, [this]() { setAlive(false); });
}

void FireWorm::updateMove(float /* dt */) {}

void FireWorm::update(float dt) {
    // Death gate
    if (m_hp <= 0.f && m_state != State::Death)
        enterDeath();
    if (m_state == State::Death) {
        m_pAnimator->update(dt);
        m_pSprite->setPosition(m_position);
        return;
    }

    updateActorBase(dt);
    updateMove(dt);

    // Animation tick
    m_pAnimator->update(dt);
    m_pSprite->setPosition(m_position);
}
