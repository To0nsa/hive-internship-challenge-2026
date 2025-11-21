#include "StatePaused.h"

#include "../ResourceManager.h"
#include "StateStack.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>

StatePaused::StatePaused(StateStack& stateStack) : m_stateStack(stateStack) {}

bool StatePaused::init() {
    m_pPrevState = m_stateStack.getCurrentState();

    const sf::Font& pFont = ResourceManager::getFont("Lavigne.ttf");

    m_pText = std::make_unique<sf::Text>(pFont);

    m_pText->setString("PRESS <ESC> TO UNPAUSE");
    m_pText->setStyle(sf::Text::Bold);
    sf::FloatRect localBounds = m_pText->getLocalBounds();
    m_pText->setOrigin({localBounds.size.x / 2.0f, localBounds.size.y / 2.0f});

    return true;
}

void StatePaused::update(float dt) {
    (void)dt;
    bool isPauseKeyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);
    m_hasPauseKeyBeenReleased |= !isPauseKeyPressed;
    if (m_hasPauseKeyBeenReleased && isPauseKeyPressed)
        m_stateStack.requestPop();
}

void StatePaused::render(sf::RenderTarget& target) const {
    if (m_pPrevState != nullptr)
        m_pPrevState->render(target);

    m_pText->setPosition({target.getSize().x * 0.5f, target.getSize().y * 0.2f});
    target.draw(*m_pText);
}
