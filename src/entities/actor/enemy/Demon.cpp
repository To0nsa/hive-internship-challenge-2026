#include "entities/actor/enemy/Demon.h"

#include "animation/Animation.h"
#include "core/Assets.h"
#include "core/ResourceManager.h"
#include "core/World.h"
#include "entities/actor/player/Player.h"
#include "gameplay/Faction.h"
#include "spell/SpellCatalog.h"
#include "spell/projectile/Projectile.h"

bool Demon::init() {
    // Load textures
    sf::Texture& flyTex   = ResourceManager::getTexture(Assets::Tex::Enemy::Demon::Fly);
    sf::Texture& deathTex = ResourceManager::getTexture(Assets::Tex::Enemy::Demon::Death);

    // Sprite
    const sf::Vector2i kFrame{81, 71};
    m_pSprite = std::make_unique<sf::Sprite>(flyTex);
    m_pSprite->setOrigin({kFrame.x * 0.5f, kFrame.y * 0.5f});
    m_spriteScale = {1.2f, 1.2f};
    applyFacingScale();
    m_pSprite->setPosition(m_position);

    // Stats
    setInitialStats(kDemonHpMax, kDemonManaMax, kDemonManaRegenRate, kDemonHpRegenRate, 0.f, 0.f);

    // Animator
    m_pAnimator = std::make_unique<SpriteAnimator>(*m_pSprite);

    // Clips
    m_flyClip =
        m_pAnimator->addClip(Animation::makeClipFromRow("fly", flyTex, kFrame, 4, 10.f, true));
    m_deathClip =
        m_pAnimator->addClip(Animation::makeClipFromRow("death", deathTex, kFrame, 7, 10.f, false));

    // Collider
    setColliderSize({kFrame.x * 0.30f, kFrame.y * 0.60f});
    setArtFacingDirX(-1.f);
    enterFly();
    return true;
}

void Demon::enterFly() {
    m_state = State::Fly;
    if (m_pAnimator)
        m_pAnimator->playClip(m_flyClip);
}

void Demon::enterDeath() {
    m_state = State::Death;
    if (m_pAnimator)
        m_pAnimator->playClip(m_deathClip, [this]() { setAlive(false); });
}

void Demon::updateFly(float dt) {
    if (!m_world)
        return;
    Player* player = m_world->getPlayer();
    if (!player)
        return;

    const sf::Vector2f toPlayer = player->getPosition() - m_position;
    const float        dist     = math::length(toPlayer);

    sf::Vector2f dir{0.f, 0.f};
    if (dist > 0.001f) {
        const sf::Vector2f n = math::normalizeVec(toPlayer);
        if (dist > kDesiredRange + kHoldSlack) {
            dir = n; // move toward
        } else if (dist < kDesiredRange - kHoldSlack) {
            dir = {-n.x, -n.y}; // move away
        }

        // Face horizontally toward player
        setFacing((toPlayer.x >= 0.f) ? Entity::Facing::Right : Entity::Facing::Left);
    }

    m_position.x += dir.x * kMoveSpeed * dt;
    m_position.y += dir.y * kMoveSpeed * dt;
}

void Demon::update(float dt) {
    // Death gate
    if (m_hp <= 0.f && m_state != State::Death)
        enterDeath();
    if (m_state == State::Death) {
        if (m_pAnimator)
            m_pAnimator->update(dt);
        if (m_pSprite)
            m_pSprite->setPosition(m_position);
        return;
    }

    updateActorBase(dt);
    updateFly(dt);

    // Casting: aim at player and fire when in range and cooldown elapsed
    if (m_castCooldownLeft > 0.f)
        m_castCooldownLeft -= dt;

    if (m_castCooldownLeft <= 0.f) {
        auto*        player = m_world->getPlayer();
        sf::Vector2f dir    = math::normalizeVec(player->getPosition() - m_position);

        // Spawn lightning bolt
        const sf::Vector2f origin = m_position + dir * 20.f;
        if (auto* proj =
                m_world->createEntity<Projectile>(SpellId::Lightning, Faction::Enemy, origin, dir))
            (void)proj->init();
        m_castCooldownLeft = kCastCooldown;
    }
    // Animation tick
    m_pAnimator->update(dt);
    m_pSprite->setPosition(m_position);
}
