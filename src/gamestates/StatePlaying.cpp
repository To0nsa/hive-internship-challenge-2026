#include "StatePlaying.h"

#include "../Config.h"
#include "../ResourceManager.h"
#include "../utils/Math.h"
#include "StatePaused.h"
#include "StateStack.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>

StatePlaying::StatePlaying(StateStack& stateStack) : m_stateStack(stateStack) {}

bool StatePlaying::init() {
    // View set to window size
    m_view.setSize(
        {static_cast<float>(Config::windowWidth), static_cast<float>(Config::windowHeight)});
    m_view.setCenter({m_view.getSize().x * 0.5f, m_view.getSize().y * 0.5f});
    // Camera state
    m_cameraX       = m_view.getCenter().x;
    m_cameraTargetX = m_cameraX;
    m_cameraSpeed   = 0.f; // ease-in

    m_ground.setSize({1024.0f, 256.0f});
    m_ground.setPosition({0.0f, 800.0f});
    m_ground.setFillColor(sf::Color::Green);

    m_pPlayer = createEntity<Player>();
    if (!m_pPlayer || !m_pPlayer->init())
        return false;
    m_pPlayer->setPosition(sf::Vector2f(200, 800));

    return true;
}

void StatePlaying::update(float dt) {
    bool isPauseKeyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);
    m_hasPauseKeyBeenReleased |= !isPauseKeyPressed;
    if (m_hasPauseKeyBeenReleased && isPauseKeyPressed) {
        m_hasPauseKeyBeenReleased = false;
        m_stateStack.push<StatePaused>();
    }

    // Update all entities
    for (const std::unique_ptr<Entity>& pEntity : m_entities)
        pEntity->update(dt);

    // CAMERA UPDATE
    // Camera auto-scroll baseline (advance target; center eases toward it)
    if (m_cameraSpeed < m_cameraTargetSpeed) {
        m_cameraSpeed = std::min(m_cameraTargetSpeed, m_cameraSpeed + kCameraAccel * dt);
    } else if (m_cameraSpeed > m_cameraTargetSpeed) {
        m_cameraSpeed = std::max(m_cameraTargetSpeed, m_cameraSpeed - kCameraAccel * dt);
    }
    m_cameraTargetX += m_cameraSpeed * dt;
    // If player passes the follow threshold, let the target drift toward player smoothly.
    if (m_pPlayer && m_pPlayer->isAlive()) {
        const float playerX         = m_pPlayer->getPosition().x;
        const float followThreshold = getFollowThresholdX();
        if (playerX > followThreshold) {
            const float alphaT    = math::expSmoothingFactor(kTargetCatchupLerp, dt);
            const float desired   = playerX; // player centered
            const float newTarget = m_cameraTargetX + (desired - m_cameraTargetX) * alphaT;
            m_cameraTargetX       = std::max(m_cameraTargetX, newTarget);
        }
    }
    // Ease camera center toward target
    const float alpha = math::expSmoothingFactor(kCatchupLerp, dt);
    m_cameraX += (m_cameraTargetX - m_cameraX) * alpha;
    m_view.setCenter({m_cameraX, m_view.getSize().y * 0.5f});
}

void StatePlaying::render(sf::RenderTarget& target) const {
    const sf::View prev = target.getView();
    target.setView(m_view);

    target.draw(m_ground);
    for (const std::unique_ptr<Entity>& pEntity : m_entities) {
        pEntity->render(target);
    }

    // Restore previous view (for UI, overlays, etc.)
    target.setView(prev);
}

float StatePlaying::getCameraLeft() const {
    return m_view.getCenter().x - 0.5f * m_view.getSize().x;
}

float StatePlaying::getCameraCatchupX() const { return getCameraLeft() + kCatchupMarginLeft; }

float StatePlaying::getFollowThresholdX() const {
    return getCameraLeft() + kFollowThresholdRatio * m_view.getSize().x;
}
