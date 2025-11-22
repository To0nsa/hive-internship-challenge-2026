#include "spell/projectile/Projectile.h"

#include "core/Config.h"
#include "core/Debug.h"
#include "core/ResourceManager.h"
#include "entities/Entity.h"
#include "utils/Math.h"

static inline AnimationClip makeClipFromSpell(const std::string& name, const SpellClip& spell) {
    const sf::Texture& tex = ResourceManager::getTexture(spell.textureKey);
    auto clip = Animation::makeClipFromRow(name, tex, spell.frameSize, spell.frameCount, spell.fps,
                                           spell.loop);
    return clip;
}

Projectile::Projectile(SpellId spellId, Faction faction, const sf::Vector2f& origin,
                       const sf::Vector2f& direction)
    : m_spellId(spellId), m_faction(faction), m_def(getSpellDef(spellId)) {
    // Direction / velocity (normalize; fallback +X)
    sf::Vector2f dir = direction;
    const float  len = math::length(dir);
    if (len > 0.f)
        dir /= len;
    else
        dir = {1.f, 0.f};
    m_velocity   = dir * m_def.stats.projectile.speed;
    m_timeToLive = m_def.stats.projectile.lifetime;

    // Sprite + animator
    const sf::Texture& pStart = ResourceManager::getTexture(m_def.visuals.start.textureKey);
    m_pSprite                 = std::make_unique<sf::Sprite>(pStart);
    m_pAnimator               = std::make_unique<SpriteAnimator>(*m_pSprite);

    // Build animation clips
    buildClips();

    // Initial placement
    m_pSprite->setPosition(origin);
    m_pSprite->setOrigin(sf::Vector2f(static_cast<float>(m_def.visuals.loop.frameSize.x) * 0.5f,
                                      static_cast<float>(m_def.visuals.loop.frameSize.y) * 0.5f));
    const float ang = std::atan2(dir.y, dir.x) * 180.f / 3.14159265f;
    m_pSprite->setRotation(sf::degrees(ang));
    m_pSprite->setScale(sf::Vector2f(2.f, 2.f));

    // Collider
    setColliderSize(m_def.stats.projectile.colliderSize);

    // Enter START phase
    enterStart();
}

void Projectile::buildClips() {
    // Create and register start/loop/hit clips
    m_startClip = m_pAnimator->addClip(makeClipFromSpell("start", m_def.visuals.start));
    m_loopClip  = m_pAnimator->addClip(makeClipFromSpell("loop", m_def.visuals.loop));
    m_hitClip   = m_pAnimator->addClip(makeClipFromSpell("hit", m_def.visuals.hit));
}

void Projectile::enterStart() {
    m_phase = Phase::Start;

    m_pAnimator->playClip(m_startClip, [this]() { enterLoop(); });
}

void Projectile::enterLoop() {
    if (m_phase == Phase::Dead)
        return;
    m_phase = Phase::Loop;
    m_pAnimator->playClip(m_loopClip);
}

void Projectile::enterHit() {
    if (m_phase == Phase::Dead)
        return;
    m_phase = Phase::Hit;

    // Stop moving during the hit
    m_velocity = {0.f, 0.f};

    // When HIT completes, mark dead
    m_pAnimator->playClip(m_hitClip, [this]() {
        m_phase = Phase::Dead;
        setAlive(false);
    });
}

void Projectile::requestImpact() {
    if (!isAlive())
        return;
    if (m_phase == Phase::Hit || m_phase == Phase::Dead)
        return;
    m_hitRequested = true;
}

void Projectile::update(float dt) {
    if (!isAlive())
        return;

    // Lifetime
    if (m_phase != Phase::Hit) {
        m_timeToLive -= dt;
        if (m_timeToLive <= 0.f) {
            enterHit();
            m_pAnimator->update(dt);
            return;
        }
    }

    // Movement only during Start/Loop
    if (m_phase == Phase::Start || m_phase == Phase::Loop) {
        if (m_pSprite)
            m_pSprite->move(m_velocity * dt);
    }

    // If an impact was requested (by World collision), switch to Hit once
    if (m_hitRequested) {
        m_hitRequested = false;
        enterHit();
    }

    // Animate
    m_pAnimator->update(dt);
}

void Projectile::render(sf::RenderTarget& t) const {
    if (m_pSprite)
        t.draw(*m_pSprite);
    if constexpr (Config::kDebugDraw) {
        Debug::drawSpriteBounds(t, *m_pSprite, sf::Color::Red, 1.f);
        Debug::drawColliderBounds(t, getCollider(), sf::Color::Green, 1.f);
    }
}
