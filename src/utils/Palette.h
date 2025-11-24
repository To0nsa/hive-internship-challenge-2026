#pragma once

#include <SFML/Graphics/Color.hpp>

namespace Palette {
    // UI elements
    inline constexpr sf::Color kUiBack{24, 24, 24, 255};       // dark gray
    inline constexpr sf::Color kUiOutline{240, 240, 240, 255}; // light gray

    // Player stats
    inline constexpr sf::Color kHpFill{220, 68, 64, 255};      // red
    inline constexpr sf::Color kManaFill{45, 152, 218, 255};   // blue
    inline constexpr sf::Color kStaminaFill{76, 209, 55, 255}; // green
} // namespace Palette
