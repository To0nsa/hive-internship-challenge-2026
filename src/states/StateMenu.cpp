#include "StateMenu.h"

#include "ResourceManager.h"
#include "StatePlaying.h"
#include "StateStack.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

StateMenu::StateMenu(StateStack& stateStack, sf::RenderWindow& window)
    : m_stateStack(stateStack), m_window(window) {}

bool StateMenu::init() {
    const sf::Font& font = ResourceManager::getFont("Lavigne.ttf");

    m_pText = std::make_unique<sf::Text>(font);

    m_pText->setString("PRESS <ENTER> TO PLAY");
    m_pText->setStyle(sf::Text::Bold);
    sf::FloatRect localBounds = m_pText->getLocalBounds();
    m_pText->setOrigin({localBounds.size.x / 2.0f, localBounds.size.y / 2.0f});

    m_pSubText = std::make_unique<sf::Text>(font);

    m_pSubText->setString("Use WASD to move\n SPACE to jump\n Mouse left click to cast a spell\n "
                          "Aim with the mouse\n Dash with mouse wheel click button\n Avoid demons "
                          "and obstacles!\n Collect red squares "
                          "and survive to gain score\n Be careful a big wave of lava is coming!");
    m_pSubText->setStyle(sf::Text::Regular);
    localBounds = m_pSubText->getLocalBounds();
    m_pSubText->setOrigin({localBounds.size.x / 2.0f, localBounds.size.y / 2.0f});

    return true;
}

void StateMenu::handleEvent(const sf::Event& event) {
    if (const auto* pKeyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (pKeyEvent->scancode == sf::Keyboard::Scan::Enter) {
            m_stateStack.push<StatePlaying>(m_window);
        }
    }
}

void StateMenu::update(float dt) { (void)dt; }

void StateMenu::render(sf::RenderTarget& target) const {
    m_pText->setPosition({target.getSize().x * 0.5f, target.getSize().y * 0.8f});
    target.draw(*m_pText);
    m_pSubText->setPosition({target.getSize().x * 0.5f, target.getSize().y * 0.2f});
    target.draw(*m_pSubText);
}
