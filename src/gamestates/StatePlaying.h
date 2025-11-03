#pragma once

#include "../entities/Entity.h"
#include "../entities/actor/player/Player.h"
#include "IState.h"
// Level visuals
#include "../level/AnimatedParallaxStrip.h"
#include "../level/ParallaxBackground.h"

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
    float getGroundTopY() const { return m_ground.getPosition().y; }

    // Camera helpers
    float getCameraLeft() const;
    float getCameraCatchupX() const;   // left edge + margin
    float getFollowThresholdX() const; // left + ratio * width

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
    // Ground
    sf::RectangleShape m_ground;

    // Camera
    sf::View m_view;
    float    m_cameraX           = 0.f;
    float    m_cameraTargetX     = 0.f; // smoothed target center X
    float    m_cameraSpeed       = 0.f;
    float    m_cameraTargetSpeed = 500.f;

    // Camera tuning
    static inline constexpr float kCameraAccel          = 1200.f;
    static inline constexpr float kCatchupMarginLeft    = 60.f;
    static inline constexpr float kFollowThresholdRatio = 0.80f; // 80% from left
    static inline constexpr float kCatchupLerp          = 8.f;   // view center smoothing
    static inline constexpr float kTargetCatchupLerp    = 2.5f;  // target center smoothing
};
