// StatBar.cpp
#include "ui/game/StatBar.h"

#include <algorithm>

StatBar::StatBar() {
    m_back.setFillColor(sf::Color::Black);
    m_back.setOutlineColor(sf::Color::White);
    m_back.setOutlineThickness(kDefaultOutline);

    m_fill.setFillColor(sf::Color::Green);

    m_back.setSize(m_size);
    updateVisual();
}

void StatBar::setValue(float current, float max) {
    float fraction = 0.f;

    if (max > 0.f) {
        fraction = current / max;

        if (fraction < 0.f) {
            fraction = 0.f;
        } else if (fraction > 1.f) {
            fraction = 1.f;
        }
    }

    setFraction(fraction);
}

void StatBar::setFraction(float fraction) {
    const float clamped = std::clamp(fraction, 0.f, 1.f);
    if (clamped == m_fraction)
        return;
    m_fraction = clamped;
    updateVisual();
}

void StatBar::setColors(const sf::Color& fill, const sf::Color& back, const sf::Color& outline) {
    m_fill.setFillColor(fill);
    m_back.setFillColor(back);
    m_back.setOutlineColor(outline);
}

void StatBar::setSize(sf::Vector2f size) {
    if (size == m_size)
        return;
    m_size = size;
    m_back.setSize(size);
    updateVisual();
}

void StatBar::setPosition(sf::Vector2f pos) {
    m_back.setPosition(pos);
    m_fill.setPosition(pos);
}

void StatBar::setOutlineThickness(float px) { m_back.setOutlineThickness(px); }

void StatBar::updateVisual() {
    const float w = std::max(0.f, m_size.x * m_fraction);
    m_fill.setSize({w, m_size.y});
}

void StatBar::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(m_back, states);
    target.draw(m_fill, states);
}
