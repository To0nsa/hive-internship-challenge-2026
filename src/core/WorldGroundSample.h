#pragma once

#include "environment/Environment.h"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/View.hpp>

// Query result when sampling ground / hazards under a horizontal band.
struct GroundSample {
    // True when there is any supporting solid (e.g., ground band) under the sampled band.
    bool hasSolid = false;

    // True when the environment hazard (lava / gap) intersects the sampled band.
    bool hasHazard = false;

    // Top Y of the main ground we want to stand on when hasSolid == true.
    float topY = 0.f;

    bool isSafe() const { return hasSolid && !hasHazard; }
};

// Stateless helper that samples ground + hazards for a given environment/view.
inline GroundSample sampleGroundFromEnvironment(const Environment& environment,
                                                const sf::View& referenceView, float xCenter,
                                                float bandWidth) {
    GroundSample sample{};

    sf::View probeView = referenceView;
    probeView.setCenter({xCenter, referenceView.getCenter().y});

    const sf::Vector2f viewSize   = probeView.getSize();
    const sf::Vector2f viewCenter = probeView.getCenter();
    const float        top        = viewCenter.y - 0.5f * viewSize.y;
    const float        bottom     = viewCenter.y + 0.5f * viewSize.y;

    if (bandWidth <= 0.f)
        bandWidth = 32.f;
    const float halfW = bandWidth * 0.5f;

    const sf::FloatRect column{{xCenter - halfW, top}, {bandWidth, bottom - top}};

    sample.hasHazard = environment.intersectsHazard(column, probeView);
    if (sample.hasHazard)
        return sample;

    const float groundTop = environment.getGroundTopY(probeView);
    sample.hasSolid       = true;
    sample.topY           = groundTop;

    return sample;
}
