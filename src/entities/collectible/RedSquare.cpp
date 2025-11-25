#include "entities/collectible/RedSquare.h"

#include "core/Assets.h"
#include "core/Config.h"
#include "core/Debug.h"
#include "core/ResourceManager.h"
#include "core/World.h"
#include "entities/actor/player/Player.h"
#include "utils/Geom.h"

bool RedSquare::init() {
    // Hidden sprite used only for collider transform (not rendered)
    const sf::Texture& tex = ResourceManager::getTexture(Assets::Tex::Spell::Icebolt::Start);
    m_pSprite              = std::make_unique<sf::Sprite>(tex);
    m_pSprite->setOrigin({kSize * 0.5f, kSize * 0.5f});
    m_pSprite->setPosition(m_startCenter);

    // Collider matches square size
    setColliderSize({kSize, kSize});

    // Visual rectangle
    m_shape.setSize({kSize, kSize});
    m_shape.setOrigin({kSize * 0.5f, kSize * 0.5f});
    m_shape.setFillColor(sf::Color(220, 68, 64)); // Red-ish
    m_shape.setOutlineColor(sf::Color::White);
    m_shape.setOutlineThickness(1.f);
    m_shape.setPosition(m_startCenter);
    return true;
}

void RedSquare::update(float dt) {
    // Rotate in place
    const float ang = m_shape.getRotation().asDegrees() + kRotateDps * dt;
    m_shape.setRotation(sf::degrees(ang));
    if (m_pSprite)
        m_pSprite->setPosition(m_shape.getPosition());
}

void RedSquare::render(sf::RenderTarget& target) const {
    target.draw(m_shape);
    if constexpr (Config::kDebugDraw) {
        Debug::drawColliderBounds(target, getCollider(), sf::Color::Green, 1.f);
    }
}
