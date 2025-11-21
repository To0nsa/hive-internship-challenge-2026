#pragma once

#include "../Config.h"
#include "../Debug.h"
#include "../ResourceManager.h"
#include "../entities/Entity.h"
#include "PlatformTypes.h"

// Minimal static-image platform entity. No animation.
class Platform final : public Entity {
  public:
    explicit Platform(const PlatformDesc& desc) : m_desc(desc) {}

    bool init() override {
        const sf::Texture& tex = ResourceManager::getTexture(m_desc.textureKey);

        m_pSprite     = std::make_unique<sf::Sprite>(tex);
        m_spriteScale = m_desc.scale;
        applyFacingScale();

        if (m_desc.frameSize.x > 0 && m_desc.frameSize.y > 0) {
            const sf::IntRect rect{{0, 0}, {m_desc.frameSize.x, m_desc.frameSize.y}};
            m_pSprite->setTextureRect(rect);
            m_pSprite->setOrigin({m_desc.frameSize.x * 0.5f, m_desc.frameSize.y * 0.5f});
        }

        // Collider setup
        sf::Vector2f spritePos = m_pSprite->getPosition();
        setColliderSize(m_desc.colliderSize);
        setColliderOffset(spritePos);
        return true;
    }

    void update(float) override {
        if (m_pSprite)
            m_pSprite->setPosition(m_position);
    }

    void render(sf::RenderTarget& target) const override {
        if (m_pSprite)
            target.draw(*m_pSprite);
        if constexpr (Config::kDebugDraw) {
            Debug::drawColliderBounds(target, getCollider(), sf::Color::Cyan, 1.f);
            Debug::drawSpriteBounds(target, *m_pSprite, sf::Color::Magenta, 1.f);
            Debug::drawSpriteOrigin(target, *m_pSprite, sf::Color::Yellow, 10.f, 2.f);
        }
    }

    const PlatformDesc& getDesc() const { return m_desc; }

  private:
    PlatformDesc m_desc;
};
