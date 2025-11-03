#pragma once

#include "../entities/Entity.h"
#include "../entities/actor/player/Player.h"
#include "IState.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/VertexArray.hpp>

class StatePlaying : public IState {
  public:
    StatePlaying(StateStack& stateStack);
    ~StatePlaying() = default;

    bool init() override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) const override;

    // World queries
    float getGroundTopY() const { return m_ground.getPosition().y; }

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

    // Ground
    sf::RectangleShape m_ground;
};
