#pragma once

#include "GameHUD.h"
#include "IState.h"
#include "World.h"
#include "entities/actor/player/Player.h"
#include "session/GameSession.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

class StatePlaying : public IState {
  public:
    StatePlaying(StateStack& stateStack, sf::RenderWindow& window);
    ~StatePlaying() = default;

    bool init() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) const override;

    // Lifecycle
    void requestExitToMenu();

  private:
    StateStack&       m_stateStack;
    sf::RenderWindow& m_window;
    GameSession       m_session;
    World             m_world;
    PlayerInput       m_input;
    GameHUD           m_hud;
};
