#pragma once

#include <array>

namespace EnvConfig {
    namespace VolcanoDay {
        // How fast the animated BG strip scrolls and how fast it animates
        inline constexpr float kBgAnimFactor = 0.15f;
        inline constexpr float kBgAnimFps    = 6.f;

        // Parallax factors for each layer, ordered to match Layer01..Layer08
        inline constexpr std::array<float, 8> kLayerFactors = {
            0.16f, // Layer01
            0.22f, // Layer02
            0.28f, // Layer03
            0.34f, // Layer04
            0.50f, // Layer05
            0.62f, // Layer06
            1.00f, // Layer07 (ground speed)
            0.76f, // Layer08 (foreground)
        };
    } // namespace VolcanoDay
} // namespace EnvConfig
