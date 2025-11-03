#pragma once

#include "IState.h"
#include "entities/Player.h"
#include "entities/Enemy.h"
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

class StatePlaying : public IState
{
public:
    StatePlaying(StateStack& stateStack);
    ~StatePlaying() = default;

    bool init() override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) const override;

private:
    static constexpr const float enemySpawnInterval = 2.0f;
    float m_timeUntilEnemySpawn = enemySpawnInterval;

    StateStack& m_stateStack;
    std::unique_ptr<Player> m_pPlayer;
    std::vector<std::unique_ptr<Enemy>> m_enemies;
    sf::RectangleShape m_ground;
    bool m_hasPauseKeyBeenReleased = true;

    void updateCollisions();
};
