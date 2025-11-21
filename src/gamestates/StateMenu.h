#pragma once

#include "IState.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <memory>

namespace sf {
    class Text;
};

class StateMenu : public IState {
  public:
    StateMenu(StateStack& stateStack, sf::RenderWindow& window);
    ~StateMenu() = default;

    bool init() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) const override;

  public:
    StateStack&               m_stateStack;
    sf::RenderWindow&         m_window;
    std::unique_ptr<sf::Text> m_pText;
    std::unique_ptr<sf::Text> m_pSubText;
};
