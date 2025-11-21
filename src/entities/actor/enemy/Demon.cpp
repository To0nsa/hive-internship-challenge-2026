#include "Demon.h"

#include "../../../ResourceManager.h"
#include "../../../animation/Animation.h"
#include "../../../entities/actor/player/Player.h"
#include "../../../faction/Faction.h"
#include "../../../World.h"
#include "../../../spell/SpellCatalog.h"
#include "../../../spell/projectile/Projectile.h"

namespace {
    constexpr const char* kFly   = "demon_fly";
    constexpr const char* kDeath = "demon_death";
} // namespace

bool Demon::init() {
    // Load textures
    sf::Texture& flyTex   = ResourceManager::getTexture("DemonFlyAnimation.png");
    sf::Texture& deathTex = ResourceManager::getTexture("DemonDeathAnimation.png");

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
    m_pAnimator->addClip(Animation::makeClipFromRow(kFly, flyTex, kFrame, 4, 10.f, true));
    m_pAnimator->addClip(Animation::makeClipFromRow(kDeath, deathTex, kFrame, 7, 10.f, false));

    // Collider
    setColliderSize({kFrame.x * 0.30f, kFrame.y * 0.60f});
    setArtFacingDirX(-1.f);
    enterFly();
    return true;
}

void Demon::enterFly() {
    m_state = State::Fly;
    if (m_pAnimator)
        m_pAnimator->ensureClip(kFly);
}

void Demon::enterDeath() {
    m_state = State::Death;
    if (m_pAnimator)
        m_pAnimator->playClip(kDeath, [this]() { setAlive(false); });
}

void Demon::updateFly(float dt) {
    if (!m_world)
        return;
    Player* player = m_world->getPlayer();
    if (!player)
        return;

    const sf::Vector2f toPlayer = player->getPosition() - m_position;
    const float        dist     = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

    sf::Vector2f dir{0.f, 0.f};
    if (dist > 0.001f) {
        const sf::Vector2f n = {toPlayer.x / dist, toPlayer.y / dist};
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
    if (m_castCooldownLeft < 0.f)
        m_castCooldownLeft = 0.f;

    if (m_world && m_castCooldownLeft <= 0.f) {
        if (auto* player = m_world->getPlayer()) {
            const sf::Vector2f selfPos = m_position;
            sf::Vector2f       dir     = player->getPosition() - selfPos;
            const float        len     = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len > 0.001f)
                dir = {dir.x / len, dir.y / len};
            else
                dir = {1.f, 0.f};

            // Spawn lightning bolt
            const sf::Vector2f origin = selfPos + dir * 20.f;
            if (auto* proj = m_world->createEntity<Projectile>(SpellId::Lightning, Faction::Enemy,
                                                               origin, dir))
                (void)proj->init();
            m_castCooldownLeft = kCastCooldown;
        }
    }

    // Animation tick
    if (m_pAnimator)
        m_pAnimator->update(dt);
    if (m_pSprite)
        m_pSprite->setPosition(m_position);
}
