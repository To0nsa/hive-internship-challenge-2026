// GameHUD.cpp
#include "GameHUD.h"

#include "../utils/Palette.h"
#include "../entities/actor/player/Player.h"

GameHUD::GameHUD() {
    // Stats Bars
    // HP bar
    m_hpBar.setColors(Palette::kHpFill, Palette::kUiBack, Palette::kUiOutline);
    m_hpBar.setSize(kBarSize);
    m_hpBar.setOutlineThickness(kOutlinePx);
    // Mana bar
    m_manaBar.setColors(Palette::kManaFill, Palette::kUiBack, Palette::kUiOutline);
    m_manaBar.setSize(kBarSize);
    m_manaBar.setOutlineThickness(kOutlinePx);
    // Stamina bar
    m_staminaBar.setColors(Palette::kStaminaFill, Palette::kUiBack, Palette::kUiOutline);
    m_staminaBar.setSize(kBarSize);
    m_staminaBar.setOutlineThickness(kOutlinePx);

    layout();
}

void GameHUD::setPlayer(const Player* player) { m_pPlayer = player; }

void GameHUD::update(float dt) {
    (void)dt;
    if (!m_pPlayer)
        return;

    m_hpBar.setValue(m_pPlayer->getHp(), m_pPlayer->getHpMax());
    m_manaBar.setValue(m_pPlayer->getMana(), m_pPlayer->getManaMax());
    m_staminaBar.setValue(m_pPlayer->getStamina(), m_pPlayer->getStaminaMax());
}

void GameHUD::setTopLeft(sf::Vector2f topLeft) {
    if (topLeft == m_topLeft)
        return;
    m_topLeft = topLeft;
    layout();
}

void GameHUD::layout() {
    const float x = m_topLeft.x;
    float       y = m_topLeft.y;

    m_hpBar.setPosition({x, y});
    y += kBarSize.y + kBarSpacing;
    m_manaBar.setPosition({x, y});
    y += kBarSize.y + kBarSpacing;
    m_staminaBar.setPosition({x, y});
}

void GameHUD::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(m_hpBar, states);
    target.draw(m_manaBar, states);
    target.draw(m_staminaBar, states);
}

