// GameHUD.cpp
#include "ui/game/GameHUD.h"

#include "core/Assets.h"
#include "core/Config.h"
#include "core/ResourceManager.h"
#include "entities/actor/player/Player.h"
#include "utils/Palette.h"
#include "utils/utils.h"

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
    const sf::Font& font = ResourceManager::getFont(Assets::Font::Lavigne);
    m_pTimerText         = std::make_unique<sf::Text>(font, "00:00", 28);
    m_pTimerText->setFillColor(Palette::kUiOutline);
    m_pTimerText->setOutlineColor(Palette::kUiBack);
    m_pTimerText->setOutlineThickness(2.f);
    // Center origin for top-center anchor
    const auto bounds = m_pTimerText->getLocalBounds();
    m_pTimerText->setOrigin(
        {bounds.size.x * 0.5f + bounds.position.x, bounds.size.y * 0.5f + bounds.position.y});
    m_pTimerText->setPosition({Config::windowWidth * 0.5f, m_timerTopY});

    // Score text (top-right)
    m_pScoreText = std::make_unique<sf::Text>(font, "Score 0", 28);
    m_pScoreText->setFillColor(Palette::kUiOutline);
    m_pScoreText->setOutlineColor(Palette::kUiBack);
    m_pScoreText->setOutlineThickness(2.f);
    const auto sb = m_pScoreText->getLocalBounds();
    // Top-right anchor
    m_pScoreText->setOrigin({sb.size.x + sb.position.x, sb.position.y});
    m_pScoreText->setPosition({static_cast<float>(Config::windowWidth) - 20.f, m_timerTopY});

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
    m_pTimerText->setOrigin(
        {bounds.size.x * 0.5f + bounds.position.x, bounds.size.y * 0.5f + bounds.position.y});
    m_pTimerText->setPosition({Config::windowWidth * 0.5f, m_timerTopY});
}

void GameHUD::setScore(int score) {
    if (!m_pScoreText)
        return;
    const std::string text = std::string("Score ") + std::to_string(score);
    if (m_pScoreText->getString() == text)
        return;
    m_pScoreText->setString(text);
    // Maintain top-right anchor when length changes
    const auto sb = m_pScoreText->getLocalBounds();
    m_pScoreText->setOrigin({sb.size.x + sb.position.x, sb.position.y});
    m_pScoreText->setPosition({static_cast<float>(Config::windowWidth) - 20.f, m_timerTopY});
}

void GameHUD::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(m_hpBar, states);
    target.draw(m_manaBar, states);
    target.draw(m_staminaBar, states);
    if (m_pTimerText)
        target.draw(*m_pTimerText, states);
    if (m_pScoreText)
        target.draw(*m_pScoreText, states);
}
