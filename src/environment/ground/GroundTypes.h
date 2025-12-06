#pragma once

#include "environment/StripUtil.h"
#include "utils/Math.h"

#include <SFML/Graphics/Rect.hpp>
#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>

// Types of hazards that can fill ground gaps.
enum class HazardType {
    None, // gaps are just empty pits, no separate hazard band
    Lava,
    Hole,
    Water,
    Spikes,
};

// A single gap instance in world space, with its originating block index.
struct GroundGap {
    sf::FloatRect rect{};
    int           blockIndex = 0;
};

struct HazardStyle {
    std::string_view texturePath{};
    float            scale   = 1.f;
    float            yOffset = 0.f;
};

struct HazardConfig {
    HazardStyle lava;              // animated lava band
    HazardStyle hole;              // simple filled gap (no texture)
    float       holeChance = 0.5f; // [0,1]: fraction of gaps rendered as holes
    bool        enabled    = false;
};

// Function type used to customize how gaps are placed inside blocks.
// Returning std::nullopt means "no gap for this block".
using GapPatternFn = std::function<std::optional<sf::FloatRect>(
    int blockIdx, float y, float h, float cellWidth, int cellsPerBlock)>;

// --- Standard gap patterns --------------------------------------------------

// "No gaps anywhere" => continuous solid ground.
inline GapPatternFn makeNoGapPattern() {
    return [](int /*blockIdx*/, float /*y*/, float /*h*/, float /*cellWidth*/,
              int /*cellsPerBlock*/) -> std::optional<sf::FloatRect> { return std::nullopt; };
}

// Default deterministic pseudo-random "one gap per block" pattern.
// - Gap width = 1 cell
// - Uses block index as seed, so layout is stable across runs.
inline GapPatternFn makeDefaultRandomGapPattern() {
    return [](int blockIdx, float y, float h, float cellWidth,
              int cellsPerBlock) -> std::optional<sf::FloatRect> {
        if (cellsPerBlock <= 1)
            return std::nullopt;

        const int anchor = blockIdx * cellsPerBlock;

        // Hash the anchor so gaps are pseudo-random but deterministic
        const std::uint32_t random = math::mix32(static_cast<std::uint32_t>(anchor));

        // Choose an offset in [1..cellsPerBlock-1] -> keeps gap away from anchor cell
        const int positions = cellsPerBlock - 1;
        const int gapOffset = 1 + static_cast<int>(random % positions);

        // X coordinate of gap's left edge (cell-based)
        const float left = (anchor + gapOffset) * cellWidth;

        // Rect representing the whole vertical band in that gap cell
        return sf::FloatRect{{left, y}, {cellWidth, h}};
    };
}

struct GroundStreamConfig {
    strip::ParallaxLayerDesc visualLayer;

    float bandHeightRatio = 0.05f; // fraction of view height used for the ground band
    float cellWidth       = 220.f; // world units per "cell"
    int   cellsPerBlock   = 5;     // cells per block (potentially 1 gap per block)

    // Always-present pattern. Default: "no gaps".
    GapPatternFn gapPattern = makeNoGapPattern();

    HazardConfig hazard; // what fills the gaps visually / as a hazard
};

namespace GroundPresets {

    inline GroundStreamConfig solidFloor(const strip::ParallaxLayerDesc& visualLayer) {
        GroundStreamConfig cfg;
        cfg.visualLayer = visualLayer;
        cfg.gapPattern  = makeNoGapPattern(); // explicit: continuous floor
        cfg.hazard      = HazardConfig{};
        return cfg;
    }

    inline GroundStreamConfig gapsWithoutHazard(const strip::ParallaxLayerDesc& visualLayer) {
        GroundStreamConfig cfg = solidFloor(visualLayer);
        cfg.gapPattern         = makeDefaultRandomGapPattern(); // now actually has gaps
        return cfg;
    }

    inline GroundStreamConfig gapsWithHazard(const strip::ParallaxLayerDesc& visualLayer,
                                             const HazardConfig&             hazard) {
        GroundStreamConfig cfg = gapsWithoutHazard(visualLayer);
        cfg.hazard             = hazard;
        return cfg;
    }

} // namespace GroundPresets
