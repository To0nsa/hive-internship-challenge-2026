#pragma once

#include "StatBar.h"

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

class Player;

class GameHUD final : public sf::Drawable {
  public:
    GameHUD();

    void setPlayer(const Player* player);

    void update(float dt);
    void setTopLeft(sf::Vector2f topLeft);

  private:
    void layout();
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    // Player reference
    const Player* m_pPlayer = nullptr;

    // Stats bar
    StatBar m_hpBar;
    StatBar m_manaBar;
    StatBar m_staminaBar;

    // Layout
    sf::Vector2f m_topLeft{20.f, 20.f};

    inline static const sf::Vector2f kBarSize{240.f, 18.f};
    inline static constexpr float    kBarSpacing = 6.f;
    inline static constexpr float    kOutlinePx  = 1.f;
};

