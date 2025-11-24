#include "core/Camera.h"

#include "entities/actor/player/Player.h"
#include "utils/Math.h"

#include <algorithm>

Camera::Camera(const sf::Vector2f& viewSize) {
    m_view.setSize(viewSize);
    m_view.setCenter({viewSize.x * 0.5f, viewSize.y * 0.5f});

    m_cameraX       = m_view.getCenter().x;
    m_cameraTargetX = m_cameraX;
    m_cameraSpeed   = 0.f; // ease-in
}

void Camera::update(float dt, const Player* player) {
    // Auto-scroll baseline (advance target; center eases toward it)
    if (m_cameraSpeed < m_cameraTargetSpeed) {
        m_cameraSpeed = std::min(m_cameraTargetSpeed, m_cameraSpeed + kCameraAccel * dt);
    } else if (m_cameraSpeed > m_cameraTargetSpeed) {
        m_cameraSpeed = std::max(m_cameraTargetSpeed, m_cameraSpeed - kCameraAccel * dt);
    }
    m_cameraTargetX += m_cameraSpeed * dt;

    // If player passes the follow threshold, let the target drift toward player smoothly.
    if (player && player->isAlive()) {
        const float playerX         = player->getPosition().x;
        const float followThreshold = followThresholdX();
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

void Camera::apply(sf::RenderTarget& target) const { target.setView(m_view); }

float Camera::left() const { return m_view.getCenter().x - 0.5f * m_view.getSize().x; }

float Camera::right(float widthMultiplier) const {
    return left() + m_view.getSize().x * widthMultiplier;
}

float Camera::bottom() const { return m_view.getCenter().y + 0.5f * m_view.getSize().y; }

float Camera::catchupX() const { return left() + kCatchupMarginLeft; }

float Camera::followThresholdX() const {
    return left() + kFollowThresholdRatio * m_view.getSize().x;
}

sf::Vector2f Camera::mapPixelToWorld(const sf::RenderWindow& window,
                                     const sf::Vector2i&     pixel) const {
    return window.mapPixelToCoords(pixel, m_view);
}
