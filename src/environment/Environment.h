#pragma once

#include "environment/AnimatedParallaxStrip.h"
#include "environment/ParallaxBackground.h"
#include "environment/ground/GroundBand.h"
#include "environment/ground/HazardLayer.h"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/View.hpp>
#include <memory>

class Environment {
  public:
    Environment() = default;

    bool initVolcanoDay();

    void update(float dt, const sf::View& view);
    void renderBackground(sf::RenderTarget& target, const sf::View& view) const;
    void renderForeground(sf::RenderTarget& target, const sf::View& view) const;

    const MultiRectCollider* getGroundCollider() const;
    float                    getGroundTopY(const sf::View& view) const;
    bool intersectsHazard(const sf::FloatRect& aabb, const sf::View& view) const;

  private:
    std::unique_ptr<ParallaxBackground>    m_bg;
    std::unique_ptr<AnimatedParallaxStrip> m_bgAnim;
    std::unique_ptr<GroundBand>            m_ground;
    std::unique_ptr<HazardLayer>           m_hazard;
    strip::ParallaxLayerDesc               m_groundLayer{};
};
