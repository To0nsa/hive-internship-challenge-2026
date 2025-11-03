#include "RedSquare.h"

#include "../../Config.h"
#include "../../Debug.h"
#include "../../ResourceManager.h"
#include "../../entities/actor/player/Player.h"
#include "../../gamestates/StatePlaying.h"
#include "../../utils/Geom.h"

bool RedSquare::init() {
    // Hidden sprite used only for collider transform (not rendered)
    const sf::Texture* pTex = ResourceManager::getOrLoadTexture("ice_boltstart.png");
    if (!pTex)
        pTex = ResourceManager::getOrLoadTexture("PlayerIdleAnimation.png");
    if (!pTex)
        return false;
    m_pSprite = std::make_unique<sf::Sprite>(*pTex);
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
    if (!isAlive())
        return;

    // Rotate in place
    const float ang = m_shape.getRotation().asDegrees() + kRotateDps * dt;
    m_shape.setRotation(sf::degrees(ang));
    if (m_pSprite)
        m_pSprite->setPosition(m_shape.getPosition());

    // Collect on player intersection
    if (m_world) {
        if (auto* player = m_world->getPlayer()) {
            if (player->isAlive()) {
                const sf::FloatRect a = getCollider().worldAabb();
                const sf::FloatRect b = player->getCollider().worldAabb();
                if (geom::aabbIntersects(a, b)) {
                    m_world->addScore(100);
                    setAlive(false);
                    return;
                }
            }
        }

        // Self-cull when far behind camera
        const float camLeft = m_world->getCameraLeft();
        const float right   = geom::right(getCollider().worldAabb());
        if (right < camLeft - 100.f) {
            setAlive(false);
            return;
        }
    }
}

void RedSquare::render(sf::RenderTarget& target) const {
    target.draw(m_shape);
    if constexpr (Config::kDebugDraw) {
        Debug::drawColliderBounds(target, getCollider(), sf::Color::Green, 1.f);
    }
}
