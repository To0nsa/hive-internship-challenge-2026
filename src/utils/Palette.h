#pragma once

#include <SFML/Graphics/Color.hpp>

namespace Palette {
    // UI elements
    inline const sf::Color kUiBack{24, 24, 24, 255}; // dark gray
    inline const sf::Color kUiOutline{240, 240, 240, 255}; // light gray

    // Player stats
    inline const sf::Color kHpFill{220, 68, 64, 255};      // red
    inline const sf::Color kManaFill{45, 152, 218, 255};   // blue
    inline const sf::Color kStaminaFill{76, 209, 55, 255}; // green
} // namespace Palette

