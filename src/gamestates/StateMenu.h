#pragma once

#include "IState.h"

#include <memory>

namespace sf {
    class Text;
};

class StateMenu : public IState {
  public:
    StateMenu(StateStack& stateStack);
    ~StateMenu() = default;

    bool init() override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) const override;
    void handleEvent(const sf::Event& event) override;

  public:
    StateStack&               m_stateStack;
    std::unique_ptr<sf::Text> m_pText;
    std::unique_ptr<sf::Text> m_pSubText;
};
