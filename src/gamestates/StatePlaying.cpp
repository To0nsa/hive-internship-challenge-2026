#include "StatePlaying.h"
#include "StatePaused.h"
#include "StateStack.h"
#include "../ResourceManager.h"
#include <memory>
#include <iostream>
#include <SFML/Graphics/RenderTarget.hpp>
#include <cmath>

StatePlaying::StatePlaying(StateStack& stateStack)
    : m_stateStack(stateStack)
{
}

bool StatePlaying::init()
{
    m_ground.setSize({1024.0f, 256.0f});
    m_ground.setPosition({0.0f, 800.0f});
    m_ground.setFillColor(sf::Color::Green);

    m_pPlayer = createEntity<Player>();
    if (!m_pPlayer || !m_pPlayer->init())
        return false;
    m_pPlayer->setPosition(sf::Vector2f(200, 800));

    return true;
}

void StatePlaying::update(float dt)
{
    bool isPauseKeyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);
    m_hasPauseKeyBeenReleased |= !isPauseKeyPressed;
    if (m_hasPauseKeyBeenReleased && isPauseKeyPressed)
    {
        m_hasPauseKeyBeenReleased = false;
        m_stateStack.push<StatePaused>();
    }

    // Update all entities
    for (const std::unique_ptr<Entity>& pEntity : m_entities)
        pEntity->update(dt);
}

void StatePlaying::render(sf::RenderTarget& target) const
{
    target.draw(m_ground);
    for (const std::unique_ptr<Entity>& pEntity : m_entities)
    {
        pEntity->render(target);
    }
}
