#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

namespace Debug {
    inline void drawSpriteBounds(sf::RenderTarget& target, const sf::Sprite& sprite,
                                 const sf::Color& color, float thickness = 1.f) {
        const sf::FloatRect bounds = sprite.getGlobalBounds();

        sf::RectangleShape outline;
        outline.setPosition(bounds.position);
        outline.setSize(bounds.size);
        outline.setFillColor(sf::Color(0, 0, 0, 0));
        outline.setOutlineColor(color);
        outline.setOutlineThickness(thickness);
        target.draw(outline);
    }

    inline void drawSpriteOrigin(sf::RenderTarget& target, const sf::Sprite& sprite,
                                 const sf::Color& color, float size = 10.f, float thickness = 2.f) {
        const sf::Vector2f originPos = sprite.getTransform().transformPoint(sprite.getOrigin());

        sf::RectangleShape crosshairH;
        crosshairH.setPosition({originPos.x - size * 0.5f, originPos.y - thickness * 0.5f});
        crosshairH.setSize({size, thickness});
        crosshairH.setFillColor(color);
        target.draw(crosshairH);

        sf::RectangleShape crosshairV;
        crosshairV.setPosition({originPos.x - thickness * 0.5f, originPos.y - size * 0.5f});
        crosshairV.setSize({thickness, size});
        crosshairV.setFillColor(color);
        target.draw(crosshairV);
    }

    inline void drawColliderBounds(sf::RenderTarget& target, const Collider& collider,
                                   const sf::Color& color, float thickness = 1.f) {
        const sf::FloatRect bounds = collider.worldAabb();

        sf::RectangleShape outline;
        outline.setPosition(bounds.position);
        outline.setSize(bounds.size);
        outline.setFillColor(sf::Color(0, 0, 0, 0));
        outline.setOutlineColor(color);
        outline.setOutlineThickness(thickness);
        target.draw(outline);
    }

    inline void drawVerticalGuide(sf::RenderTarget& target, const sf::View& view, float x,
                                  const sf::Color& color, float thickness = 3.f) {
        const float        bottomY = 0.f;
        const float        height  = view.getSize().y;
        sf::RectangleShape line({thickness, height});
        line.setPosition({x - thickness * 0.5f, bottomY});
        line.setFillColor(color);
        target.draw(line);
    }

    inline void drawCameraGuides(sf::RenderTarget& target, const sf::View& view, float catchupX,
                                 float followX) {
        drawVerticalGuide(target, view, catchupX, sf::Color::Cyan);
        drawVerticalGuide(target, view, followX, sf::Color::Yellow);
    }
} // namespace Debug
