#pragma once

#include "gameplay/GameSession.h"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>
#include <memory>
#include <vector>

class AnimatedParallaxStrip;
class Entity;
class GameSession;
class GroundStream;
class ParallaxBackground;
class Player;
class StatePlaying;

namespace sf {
    class RenderTarget;
    class RenderWindow;
} // namespace sf

class World {
  public:
    World(StatePlaying& owner, GameSession& session, sf::RenderWindow& window);
    ~World();

    bool init();
    void update(float dt);
    void render(sf::RenderTarget& target) const;

    Player* getPlayer() const { return m_pPlayer; }

    float getCameraLeft() const;
    float getCameraCatchupX() const;
    float getFollowThresholdX() const;
    float getViewBottomY() const { return m_view.getCenter().y + 0.5f * m_view.getSize().y; }

    const std::vector<sf::FloatRect>& getGroundRects() const;
    const std::vector<sf::FloatRect>& getObstacleRects() const;
    const std::vector<sf::FloatRect>& getPlatformRects() const;
    const std::vector<sf::FloatRect>& getSolidTopRects() const;

    sf::Vector2f getMouseWorld() const;

    void addScore(int points);
    void requestExitToMenu();

    template <typename T, typename... Args> T* createEntity(Args&&... args) {
        auto entity    = std::make_unique<T>(std::forward<Args>(args)...);
        T*   entityPtr = entity.get();
        entityPtr->setWorld(this);
        m_entities.push_back(std::move(entity));
        return entityPtr;
    }

  private:
    StatePlaying&     m_owner;
    GameSession&      m_session;
    sf::RenderWindow& m_window;

    Player* m_pPlayer = nullptr;

    std::vector<std::unique_ptr<Entity>> m_entities;

    std::unique_ptr<ParallaxBackground>    m_bg;
    std::unique_ptr<AnimatedParallaxStrip> m_bgAnim;
    std::unique_ptr<GroundStream>          m_ground;

    float m_nextObstacleX = 0.f;
    float m_nextPlatformX = 0.f;

    sf::View m_view;
    float    m_cameraX           = 0.f;
    float    m_cameraTargetX     = 0.f; // smoothed target center X
    float    m_cameraSpeed       = 0.f;
    float    m_cameraTargetSpeed = 450.f;

    float m_demonSpawnTimer = 10.f; // spawn a demon every 10 seconds

    mutable std::vector<sf::FloatRect> m_cachedObstacleRects;
    mutable std::vector<sf::FloatRect> m_cachedPlatformRects;
    mutable std::vector<sf::FloatRect> m_cachedSolidTopRects;

    static constexpr float kCameraAccel          = 1200.f;
    static constexpr float kCatchupMarginLeft    = 60.f;
    static constexpr float kFollowThresholdRatio = 0.80f; // 80% from left
    static constexpr float kCatchupLerp          = 8.f;   // view center smoothing
    static constexpr float kTargetCatchupLerp    = 2.5f;  // target center smoothing
};
