#pragma once
#include "collision/Collider.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

namespace Debug {
    inline void drawRectOutline(sf::RenderTarget& target, const sf::FloatRect& rect,
                                const sf::Color& color, float thickness = 1.f) {
        sf::RectangleShape outline;
        outline.setPosition(rect.position);
        outline.setSize(rect.size);
        outline.setFillColor(sf::Color(0, 0, 0, 0));
        outline.setOutlineColor(color);
        outline.setOutlineThickness(thickness);
        target.draw(outline);
    }

    inline void drawSpriteBounds(sf::RenderTarget& target, const sf::Sprite& sprite,
                                 const sf::Color& color, float thickness = 1.f) {
        const sf::FloatRect bounds = sprite.getGlobalBounds();

        drawRectOutline(target, bounds, color, thickness);
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

        drawRectOutline(target, bounds, color, thickness);
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

    // Draws vertical bands across the camera view using a caller-provided sampler.
    // The sampler must be invocable as sampler(xCenter, width) and return an object
    // with bool hasSolid and bool hasHazard fields (e.g. GroundSample from WorldGroundSample.h).
    template <typename Sampler>
    inline void drawGroundSampleBands(sf::RenderTarget& target, const sf::View& view,
                                      Sampler&& sampler, float step = 96.f) {
        const sf::Vector2f viewSize   = view.getSize();
        const sf::Vector2f viewCenter = view.getCenter();
        const float        viewLeft   = viewCenter.x - 0.5f * viewSize.x;
        const float        viewTop    = viewCenter.y - 0.5f * viewSize.y;
        const float        viewRight  = viewLeft + viewSize.x;

        for (float x = viewLeft; x < viewRight; x += step) {
            const auto sample = sampler(x, step);

            sf::FloatRect band{{x - step * 0.5f, viewTop}, {step, viewSize.y}};

            sf::Color color;
            if (!sample.hasSolid)
                color = sf::Color::Red; // red: no solid
            else if (sample.hasHazard)
                color = sf::Color::Yellow; // yellow: hazard
            else
                color = sf::Color::Green; // green: safe

            drawRectOutline(target, band, color, 1.f);
        }
    }
} // namespace Debug
