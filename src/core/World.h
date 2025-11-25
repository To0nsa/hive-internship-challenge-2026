#pragma once

#include "core/Camera.h"
#include "environment/Environment.h"
#include "gameplay/GameSession.h"
#include "collision/CollisionSystem.h"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <memory>
#include <vector>

class Entity;
class GameSession;
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
    float getCameraRight() const;
    float getCameraCatchupX() const;
    float getFollowThresholdX() const;
    float getViewBottomY() const { return m_camera.bottom(); }

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

    Environment m_environment;

    float m_nextObstacleX = 0.f;
    float m_nextPlatformX = 0.f;

    Camera m_camera;

    float m_demonSpawnTimer = 10.f; // spawn a demon every 10 seconds

    CollisionContext buildCollisionContext(float dt, const MultiRectCollider* groundCollider);
};
