#pragma once

#include "../entities/Entity.h"
#include "../entities/actor/player/Player.h"
#include "../level/AnimatedParallaxStrip.h"
#include "../level/GroundStream.h"
#include "../level/ParallaxBackground.h"
#include "IState.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/View.hpp>

class StatePlaying : public IState {
  public:
    StatePlaying(StateStack& stateStack);
    ~StatePlaying() = default;

    bool init() override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) const override;

    // World queries
    float getGroundTopY() const {
        if (m_ground)
            return m_ground->getTopYForView(m_view);
        // Fallback if ground not ready yet: bottom band at 5% height
        return m_view.getCenter().y - 0.5f * m_view.getSize().y +
               (m_view.getSize().y * (1.f - 0.05f));
    }

    // Expose ground solids for simple collision in Player
    const std::vector<sf::FloatRect>& getGroundRects() const {
        if (m_ground)
            return m_ground->getCollider().getRectColliders();
        static const std::vector<sf::FloatRect> kEmpty;
        return kEmpty;
    }

    // Solid rectangles for obstacles
    const std::vector<sf::FloatRect>& getObstacleRects() const;

    // All walkable-top solids
    const std::vector<sf::FloatRect>& getSolidTopRects() const;

    // Camera helpers
    float getCameraLeft() const;
    float getCameraCatchupX() const;   // left edge + margin
    float getFollowThresholdX() const; // left + ratio * width
    float getViewBottomY() const { return m_view.getCenter().y + 0.5f * m_view.getSize().y; }

    // Create a new entity of type T and add it to the world
    template <typename T, typename... Args> T* createEntity(Args&&... args) {
        auto entity    = std::make_unique<T>(std::forward<Args>(args)...);
        T*   entityPtr = entity.get();
        entityPtr->setWorld(this);
        m_entities.push_back(std::move(entity));
        return entityPtr;
    }

  private:
    // Game state
    StateStack& m_stateStack;
    bool        m_hasPauseKeyBeenReleased = true;

    // Player reference
    Player* m_pPlayer = nullptr;

    // Entities
    std::vector<std::unique_ptr<Entity>> m_entities;

    // Backgrounds / ground
    std::unique_ptr<ParallaxBackground>    m_bg;
    std::unique_ptr<AnimatedParallaxStrip> m_bgAnim;
    std::unique_ptr<GroundStream>          m_ground;

    // Obstacle spawning
    float m_nextObstacleX = 0.f;

    // Camera
    sf::View m_view;
    float    m_cameraX           = 0.f;
    float    m_cameraTargetX     = 0.f; // smoothed target center X
    float    m_cameraSpeed       = 0.f;
    float    m_cameraTargetSpeed = 10.f;

    // Camera tuning
    static inline constexpr float kCameraAccel          = 1200.f;
    static inline constexpr float kCatchupMarginLeft    = 60.f;
    static inline constexpr float kFollowThresholdRatio = 0.80f; // 80% from left
    static inline constexpr float kCatchupLerp          = 8.f;   // view center smoothing
    static inline constexpr float kTargetCatchupLerp    = 2.5f;  // target center smoothing

    // Cached rectangles for obstacle/solid queries (rebuilt on demand)
    mutable std::vector<sf::FloatRect> m_cachedObstacleRects;
    mutable std::vector<sf::FloatRect> m_cachedSolidTopRects;
};
