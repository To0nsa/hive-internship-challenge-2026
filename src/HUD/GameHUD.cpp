// GameHUD.cpp
#include "GameHUD.h"

#include "../utils/Palette.h"
#include "../Config.h"
#include "../ResourceManager.h"
#include "../utils/utils.h"
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

    // Timer text
    if (const sf::Font* font = ResourceManager::getOrLoadFont("Lavigne.ttf")) {
        m_pTimerText = std::make_unique<sf::Text>(*font, "00:00", 28);
        m_pTimerText->setFillColor(Palette::kUiOutline);
        m_pTimerText->setOutlineColor(Palette::kUiBack);
        m_pTimerText->setOutlineThickness(2.f);
        // Center origin for top-center anchor
        const auto bounds = m_pTimerText->getLocalBounds();
        m_pTimerText->setOrigin({bounds.size.x * 0.5f + bounds.position.x,
                                 bounds.size.y * 0.5f + bounds.position.y});
        m_pTimerText->setPosition({Config::windowWidth * 0.5f, m_timerTopY});
    }

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

void GameHUD::setElapsedSeconds(float seconds) {
    if (!m_pTimerText)
        return;
    const std::string text = formatMMSS(seconds);
    if (m_pTimerText->getString() == text)
        return;
    m_pTimerText->setString(text);
    const auto bounds = m_pTimerText->getLocalBounds();
    m_pTimerText->setOrigin({bounds.size.x * 0.5f + bounds.position.x,
                             bounds.size.y * 0.5f + bounds.position.y});
    m_pTimerText->setPosition({Config::windowWidth * 0.5f, m_timerTopY});
}

void GameHUD::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(m_hpBar, states);
    target.draw(m_manaBar, states);
    target.draw(m_staminaBar, states);
    if (m_pTimerText)
        target.draw(*m_pTimerText, states);
}
