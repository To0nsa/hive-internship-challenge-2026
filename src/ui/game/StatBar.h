#pragma once
#include <SFML/Graphics.hpp>

class StatBar : public sf::Drawable {
  public:
    StatBar();

    void setValue(float current, float max);
    void setFraction(float fraction);

    // Style & layout
    void setColors(const sf::Color& fill, const sf::Color& back, const sf::Color& outline);
    void setSize(sf::Vector2f size);
    void setPosition(sf::Vector2f pos);
    void setOutlineThickness(float px);

  private:
    void updateVisual();
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    inline static const sf::Vector2f kDefaultSize{240.f, 18.f};
    inline static const float        kDefaultOutline = 1.f;

    sf::RectangleShape m_back;
    sf::RectangleShape m_fill;

    float        m_fraction = 1.f;
    sf::Vector2f m_size     = kDefaultSize;
};
