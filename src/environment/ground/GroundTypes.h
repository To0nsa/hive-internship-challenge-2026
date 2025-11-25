#pragma once

#include "environment/StripUtil.h"

#include <SFML/Graphics/Rect.hpp>
#include <functional>
#include <optional>
#include <string_view>

// Types of hazards that can fill ground gaps.
enum class HazardType {
    None, // gaps are just empty pits, no separate hazard band
    Lava,
    Water,
    Spikes,
};

struct HazardConfig {
    HazardType      type        = HazardType::None;
    std::string_view texturePath{};
    float           scale       = 1.f;
    float           yOffset     = 0.f;
};

// Function type used to customize how gaps are placed inside blocks.
// Returning std::nullopt means "no gap for this block".
using GapPatternFn = std::function<std::optional<sf::FloatRect>(
    int blockIdx, float y, float h, float cellWidth, int cellsPerBlock)>;

struct GroundStreamConfig {
    strip::ParallaxLayerDesc visualLayer;

    float bandHeightRatio = 0.05f; // fraction of view height used for the ground band
    float cellWidth       = 220.f; // world units per "cell"
    int   cellsPerBlock   = 5;     // cells per block (potentially 1 gap per block)

    bool hasGaps = true; // false => continuous solid ground, no holes

    GapPatternFn gapPattern; // optional custom gap layout; default pattern if empty

    HazardConfig hazard; // what fills the gaps visually / as a hazard
};

namespace GroundPresets {

    inline GroundStreamConfig solidFloor(const strip::ParallaxLayerDesc& visualLayer,
                                         float bandHeightRatio = 0.05f, float cellWidth = 220.f,
                                         int cellsPerBlock = 5) {
        GroundStreamConfig cfg;
        cfg.visualLayer     = visualLayer;
        cfg.bandHeightRatio = bandHeightRatio;
        cfg.cellWidth       = cellWidth;
        cfg.cellsPerBlock   = cellsPerBlock;
        cfg.hasGaps         = false;
        cfg.hazard          = HazardConfig{};
        return cfg;
    }

    inline GroundStreamConfig gapsWithoutHazard(const strip::ParallaxLayerDesc& visualLayer,
                                                float bandHeightRatio = 0.05f,
                                                float cellWidth       = 220.f,
                                                int   cellsPerBlock   = 5) {
        GroundStreamConfig cfg =
            solidFloor(visualLayer, bandHeightRatio, cellWidth, cellsPerBlock);
        cfg.hasGaps = true;
        return cfg;
    }

    inline GroundStreamConfig gapsWithHazard(const strip::ParallaxLayerDesc& visualLayer,
                                             const HazardConfig&             hazard,
                                             float bandHeightRatio = 0.05f,
                                             float cellWidth       = 220.f,
                                             int   cellsPerBlock   = 5) {
        GroundStreamConfig cfg =
            gapsWithoutHazard(visualLayer, bandHeightRatio, cellWidth, cellsPerBlock);
        cfg.hazard = hazard;
        return cfg;
    }

} // namespace GroundPresets

