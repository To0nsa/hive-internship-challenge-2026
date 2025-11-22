#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>

namespace strip {

    struct ParallaxLayerDesc {
        std::string key;
        float       factor = 0.f;
    };

    // Draw a horizontally repeating strip aligned to the view's bottom edge.
    inline void drawStrip(sf::RenderTarget& target, const sf::View& view,
                          const sf::Texture& texture, float parallaxFactor) {
        // View geometry
        const float viewWidthWorld  = view.getSize().x;
        const float viewHeightWorld = view.getSize().y;
        const float viewLeftWorld   = view.getCenter().x - 0.5f * viewWidthWorld;
        const float viewTopWorld    = 0.f; // always anchored to bottom

        // Slight overdraw to avoid edge artifacts
        constexpr float paddingPixels = 2.f;

        // Screen-space quad we'll draw into
        sf::RectangleShape stripQuad({viewWidthWorld + 2.f * paddingPixels, viewHeightWorld});
        stripQuad.setPosition({viewLeftWorld - paddingPixels, viewTopWorld});
        stripQuad.setTexture(&texture, true);

        // Texture info
        const sf::Vector2u textureSizePx   = texture.getSize();
        const int          textureHeightPx = static_cast<int>(textureSizePx.y);

        // Convert world/screen size to texture space using a uniform vertical scale
        const float pixelsPerTexel = viewHeightWorld / static_cast<float>(textureHeightPx);

        // Horizontal scrolling in texture space
        const float texUStartF = (viewLeftWorld * parallaxFactor) / pixelsPerTexel;
        const float texUWidthF = stripQuad.getSize().x / pixelsPerTexel;

        // Bottom-aligned vertical slice in texture space
        const int texVHeight = static_cast<int>(viewHeightWorld / pixelsPerTexel);
        const int texVStart  = textureHeightPx - texVHeight;

        // Truncate to ints for the texture rect
        const int texUStart = static_cast<int>(texUStartF);
        const int texUWidth = static_cast<int>(texUWidthF);

        sf::IntRect texRect({texUStart, texVStart}, {texUWidth, texVHeight});
        stripQuad.setTextureRect(texRect);
        target.draw(stripQuad);
    }

} // namespace strip
