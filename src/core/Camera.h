#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>

class Player;

// Camera owns the sf::View and encapsulates scrolling / follow logic.
class Camera {
  public:
    explicit Camera(const sf::Vector2f& viewSize);
    Camera() = default;

    void update(float dt, const Player* player);

    void            apply(sf::RenderTarget& target) const;
    const sf::View& getView() const { return m_view; }

    float left() const;
    float bottom() const;
    float catchupX() const;
    float followThresholdX() const;
    float right(float widthMultiplier = 2.f) const;

    sf::Vector2f mapPixelToWorld(const sf::RenderWindow& window, const sf::Vector2i& pixel) const;

  private:
    sf::View m_view;

    float m_cameraX           = 0.f;
    float m_cameraTargetX     = 0.f; // smoothed target center X
    float m_cameraSpeed       = 0.f;
    float m_cameraTargetSpeed = 10.f;

    static constexpr float kCameraAccel          = 1200.f;
    static constexpr float kCatchupMarginLeft    = 60.f;
    static constexpr float kFollowThresholdRatio = 0.80f; // 80% from left
    static constexpr float kCatchupLerp          = 8.f;   // view center smoothing
    static constexpr float kTargetCatchupLerp    = 2.5f;  // target center smoothing
};
