#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Event.hpp>

namespace sf {
    class RenderTarget;
};

class StateStack;

class IState {
  public:
    virtual ~IState() = default;

    virtual bool init()                                 = 0;
    virtual void handleEvent(const sf::Event& event)    = 0;
    virtual void update(float dt)                       = 0;
    virtual void render(sf::RenderTarget& target) const = 0;
};
