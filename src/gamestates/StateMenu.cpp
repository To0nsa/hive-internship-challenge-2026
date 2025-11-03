#include "StateMenu.h"

#include "../ResourceManager.h"
#include "StatePlaying.h"
#include "StateStack.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>

StateMenu::StateMenu(StateStack& stateStack) : m_stateStack(stateStack) {}

bool StateMenu::init() {
    const sf::Font* pFont = ResourceManager::getOrLoadFont("Lavigne.ttf");
    if (pFont == nullptr)
        return false;

    m_pText = std::make_unique<sf::Text>(*pFont);
    if (!m_pText)
        return false;

    m_pText->setString("PRESS <ENTER> TO PLAY");
    m_pText->setStyle(sf::Text::Bold);
    sf::FloatRect localBounds = m_pText->getLocalBounds();
    m_pText->setOrigin({localBounds.size.x / 2.0f, localBounds.size.y / 2.0f});

    m_pSubText = std::make_unique<sf::Text>(*pFont);
    if (!m_pSubText)
        return false;

    m_pSubText->setString("Use WASD to move\n SPACE to jump\n Mouse left click to cast a spell\n "
                          "Aim with the mouse\n Dash with mouse wheel click button\n Avoid demons "
                          "and obstacles!\n Collect red squares "
                          "and survive to gain score\n Be careful a big wave of lava is coming!");
    m_pSubText->setStyle(sf::Text::Regular);
    localBounds = m_pSubText->getLocalBounds();
    m_pSubText->setOrigin({localBounds.size.x / 2.0f, localBounds.size.y / 2.0f});

    return true;
}

void StateMenu::update(float dt) {
    (void)dt;
    m_hasStartKeyBeenPressed |= sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
    if (m_hasStartKeyBeenReleased) {
        m_hasStartKeyBeenPressed  = false;
        m_hasStartKeyBeenReleased = false;
        m_stateStack.push<StatePlaying>();
    }
    m_hasStartKeyBeenReleased |=
        m_hasStartKeyBeenPressed && !sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
}

void StateMenu::render(sf::RenderTarget& target) const {
    m_pText->setPosition({target.getSize().x * 0.5f, target.getSize().y * 0.8f});
    target.draw(*m_pText);
    m_pSubText->setPosition({target.getSize().x * 0.5f, target.getSize().y * 0.2f});
    target.draw(*m_pSubText);
}
