#pragma once
#include "core/ResourceManager.h"
#include "environment/BackgroundAssets.h"
#include "environment/StripUtil.h"

#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>
#include <vector>

class AnimatedParallaxStrip : public sf::Drawable {
  public:
    AnimatedParallaxStrip(std::vector<std::string> frames, float factor, float fps)
        : m_frames(std::move(frames)), m_factor(factor), m_fps(fps) {}

    void update(float dt) { m_time += dt; }

    void drawForView(sf::RenderTarget& t, const sf::View& view) const {
        if (m_frames.empty())
            return;
        const std::size_t idx  = static_cast<std::size_t>(m_time * m_fps) % m_frames.size();
        const std::string file = bgassets::keyToFilename(m_frames[idx]);
        sf::Texture&      tex  = ResourceManager::getTexture(file);
        // Ensure horizontal repeating for scrolling
        tex.setRepeated(true);
        strip::drawStrip(t, view, tex, m_factor);
    }

  private:
    void draw(sf::RenderTarget&, sf::RenderStates) const override {}

    std::vector<std::string> m_frames;
    float                    m_factor = 0.15f;
    float                    m_fps    = 2.f;
    float                    m_time   = 0.f;
};
