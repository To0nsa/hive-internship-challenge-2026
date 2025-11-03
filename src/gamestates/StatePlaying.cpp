#include "StatePlaying.h"

#include "../Config.h"
#include "../Debug.h"
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

    // Backgrounds
    m_bg = std::make_unique<ParallaxBackground>(std::initializer_list<strip::ParallaxLayerDesc>{
        {"bg_01", 0.16f},
        {"bg_02", 0.22f},
        {"bg_03", 0.28f},
        {"bg_04", 0.34f},
        {"bg_05", 0.50f},
        {"bg_06", 0.62f},
        {"bg_08", 0.76f},
    });
    // Animated background strip
    m_bgAnim = std::make_unique<AnimatedParallaxStrip>(
        std::vector<std::string>{"bg_anim_01", "bg_anim_02", "bg_anim_03"}, 0.15f, 6.f);

    // Ground
    m_ground.setSize({1024.0f, 256.0f});
    m_ground.setPosition({0.0f, 800.0f});
    m_ground.setFillColor(sf::Color(64, 160, 64));

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

    // Update background anim
    if (m_bgAnim)
        m_bgAnim->update(dt);

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
    const sf::View oldView = target.getView();
    target.setView(m_view);

    if (m_bgAnim)
        m_bgAnim->drawForView(target, m_view);

    if (m_bg) {
        const std::size_t i06  = m_bg->findIndexByKey("bg_06");
        const std::size_t upto = (i06 < m_bg->size()) ? i06 : (m_bg->size() ? m_bg->size() - 1 : 0);
        m_bg->drawRangeForView(target, m_view, 0, upto);
    }

    target.draw(m_ground);

    for (const std::unique_ptr<Entity>& pEntity : m_entities)
        pEntity->render(target);

    // Foreground layer 08 after entities
    if (m_bg) {
        const std::size_t i08 = m_bg->findIndexByKey("bg_08");
        if (i08 < m_bg->size())
            m_bg->drawRangeForView(target, m_view, i08, i08);
    }

    // Debug helpers
    if constexpr (Config::kDebugDraw) {
        Debug::drawCameraGuides(target, m_view, getCameraCatchupX(), getFollowThresholdX());
    }

    target.setView(oldView);
}

float StatePlaying::getCameraLeft() const {
    return m_view.getCenter().x - 0.5f * m_view.getSize().x;
}

float StatePlaying::getCameraCatchupX() const { return getCameraLeft() + kCatchupMarginLeft; }

float StatePlaying::getFollowThresholdX() const {
    return getCameraLeft() + kFollowThresholdRatio * m_view.getSize().x;
}
