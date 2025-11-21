#include "StatePlaying.h"

#include "StatePaused.h"
#include "StateStack.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Mouse.hpp>

StatePlaying::StatePlaying(StateStack& stateStack, sf::RenderWindow& window)
    : m_stateStack(stateStack), m_window(window), m_world(*this, m_session, window) {}

void StatePlaying::requestExitToMenu() {
    // Pop this state; the menu remains underneath.
    m_stateStack.requestPop();
}

bool StatePlaying::init() {
    m_session.start();
    if (!m_world.init())
        return false;

    if (auto* player = m_world.getPlayer())
        m_hud.setPlayer(player);
    return true;
}

void StatePlaying::handleEvent(const sf::Event& event) {
    if (const auto* pKeyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (pKeyEvent->scancode == sf::Keyboard::Scan::Escape) {
            m_stateStack.push<StatePaused>();
        }
        switch (pKeyEvent->scancode) {
        case sf::Keyboard::Scan::A:
        case sf::Keyboard::Scan::Left:
            m_input.moveLeft = true;
            break;
        case sf::Keyboard::Scan::D:
        case sf::Keyboard::Scan::Right:
            m_input.moveRight = true;
            break;
        case sf::Keyboard::Scan::Space:
            m_input.jumpPressed = true;
            break;
        default:
            break;
        }
    } else if (const auto* pKeyReleased = event.getIf<sf::Event::KeyReleased>()) {
        switch (pKeyReleased->scancode) {
        case sf::Keyboard::Scan::A:
        case sf::Keyboard::Scan::Left:
            m_input.moveLeft = false;
            break;
        case sf::Keyboard::Scan::D:
        case sf::Keyboard::Scan::Right:
            m_input.moveRight = false;
            break;
        default:
            break;
        }
    } else if (const auto* pMouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (pMouseEvent->button == sf::Mouse::Button::Middle) {
            m_input.dashPressed = true;
        } else if (pMouseEvent->button == sf::Mouse::Button::Left) {
            m_input.castPressed = true;
        }
    }
}

void StatePlaying::update(float dt) {
    m_session.update(dt);
    m_session.addScorePerSecond(dt, 10);

    if (auto* player = m_world.getPlayer(); player && player->isAlive()) {
        player->setInput(m_input);
    }

    m_world.update(dt);

    m_hud.update(dt);
    m_hud.setElapsedSeconds(m_session.elapsedSeconds());
    m_hud.setScore(m_session.score());

    // One-shot actions get cleared once consumed by update
    m_input.jumpPressed = false;
    m_input.dashPressed = false;
    m_input.castPressed = false;
}

void StatePlaying::render(sf::RenderTarget& target) const {
    m_world.render(target);
    target.draw(m_hud);
}
