#pragma once

#include "animation/Animation.h"
#include "collision/CollisionLayers.h"
#include "core/Config.h"
#include "core/Debug.h"
#include "core/ResourceManager.h"
#include "entities/Entity.h"
#include "entities/obstacle/ObstacleTypes.h"

#include <SFML/Graphics/Sprite.hpp>

// Minimal obstacle entity: static sprite with optional sheet-based animation.
class Obstacle final : public Entity {
  public:
    explicit Obstacle(const ObstacleDesc& desc) : m_desc(desc) {
        setCollisionLayer(CollisionLayer::Obstacle);
        setCollisionMask(maskFrom({CollisionLayer::Player}));
    }

    bool init() override {
        // Load texture
        const sf::Texture& tex = ResourceManager::getTexture(m_desc.textureKey);

        // Sprite setup
        m_pSprite     = std::make_unique<sf::Sprite>(tex);
        m_spriteScale = m_desc.scale;
        applyFacingScale();
        m_pSprite->setOrigin({m_desc.frameSize.x * 0.5f, m_desc.frameSize.y * 0.5f});

        // Animation setup
        if (m_desc.animated) {
            m_pAnimator = std::make_unique<SpriteAnimator>(*m_pSprite);
            auto clip =
                Animation::makeClipFromSheet("main", tex, m_desc.frameSize, m_desc.startCell,
                                             m_desc.endCell, m_desc.fps, m_desc.loop);
            m_mainClip = m_pAnimator->addClip(std::move(clip));
            m_pAnimator->playClip(m_mainClip);
        }

        // Collider setup
        sf::Vector2f spritePos = m_pSprite->getPosition();
        setColliderSize(m_desc.colliderSize);
        setColliderOffset(spritePos);

        return true;
    }

    void update(float dt) override {
        if (m_pAnimator)
            m_pAnimator->update(dt);
        if (m_pSprite)
            m_pSprite->setPosition(m_position);
    }

    void render(sf::RenderTarget& target) const override {
        if (m_pSprite)
            target.draw(*m_pSprite);
        if constexpr (Config::kDebugDraw) {
            Debug::drawColliderBounds(target, getCollider(), sf::Color::Green, 1.f);
            Debug::drawSpriteOrigin(target, *m_pSprite, sf::Color::Yellow, 10.f, 2.f);
            Debug::drawSpriteBounds(target, *m_pSprite, sf::Color::Red, 1.f);
        }
    }

    float               getDps() const { return m_desc.dps; }
    const ObstacleDesc& getDesc() const { return m_desc; }

  private:
    ObstacleDesc           m_desc;
    SpriteAnimator::ClipId m_mainClip = SpriteAnimator::kInvalidClip;
};
