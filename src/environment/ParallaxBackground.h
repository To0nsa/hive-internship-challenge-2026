#pragma once

#include "core/ResourceManager.h"
#include "environment/StripUtil.h"

#include <SFML/Graphics.hpp>
#include <vector>

class ParallaxBackground : public sf::Drawable {
  public:
    ParallaxBackground(std::initializer_list<strip::ParallaxLayerDesc> layers) : m_layers(layers) {}

    void add(const strip::ParallaxLayerDesc& desc) { m_layers.push_back(desc); }

    void drawRangeForView(sf::RenderTarget& t, const sf::View& view, std::size_t first,
                          std::size_t last) const {
        if (m_layers.empty())
            return;
        if (first >= m_layers.size())
            return;
        const std::size_t lastClamped = std::min(last, m_layers.size() - 1);
        for (std::size_t i = first; i <= lastClamped; ++i) {
            const auto&  layer = m_layers[i];
            sf::Texture& tex   = ResourceManager::getTexture(layer.texturePath);
            // Ensure horizontal repeating for scrolling
            tex.setRepeated(true);
            strip::drawStrip(t, view, tex, layer.factor);
        }
    }

    std::size_t size() const { return m_layers.size(); }

  private:
    void draw(sf::RenderTarget&, sf::RenderStates) const override {}

    std::vector<strip::ParallaxLayerDesc> m_layers;
};
