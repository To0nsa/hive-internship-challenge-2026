#pragma once

#include "Config.h"
#include "StateStack.h"

#include <SFML/Graphics/RenderWindow.hpp>

class Application {
  public:
    Application();
    int run();

  private:
    void processEvents();
    void update(float dt);
    void render();

    sf::RenderWindow m_window;
    StateStack       m_states;
};
