#include "core/Application.h"

#include "states/IState.h"
#include "states/StateMenu.h"

#include <SFML/System/Clock.hpp>
#include <optional>

Application::Application()
    : m_window(sf::VideoMode({Config::windowWidth, Config::windowHeight}), Config::windowTitle),
      m_states() {
    m_window.setKeyRepeatEnabled(false);
    m_window.setVerticalSyncEnabled(true);

    m_states.push<StateMenu>(m_window);
}

int Application::run() {
    sf::Clock clock;

    while (m_window.isOpen()) {
        const float dt = clock.restart().asSeconds();
        processEvents();
        update(dt);
        render();
        m_states.performPendingPops();
    }
    return 0;
}

void Application::processEvents() {
    IState* state = m_states.getCurrentState();
    while (const std::optional<sf::Event> event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            m_window.close();
        }
        if (state) {
            state->handleEvent(*event);
        }
    }
}

void Application::update(float dt) {
    if (IState* state = m_states.getCurrentState()) {
        state->update(dt);
    } else {
        m_window.close();
    }
}

void Application::render() {
    m_window.clear();
    if (IState* state = m_states.getCurrentState()) {
        state->render(m_window);
    }
    m_window.display();
}
