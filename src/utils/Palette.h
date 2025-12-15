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

    // Debug: static world geometry (tops)
    inline constexpr sf::Color kDebugStaticGroundTop{0, 255, 0, 255};     // bright green
    inline constexpr sf::Color kDebugStaticPlatformTop{0, 255, 255, 255}; // cyan
    inline constexpr sf::Color kDebugStaticObstacleTop{255, 0, 255, 255}; // magenta

    // Debug: static world geometry (walls)
    inline constexpr sf::Color kDebugStaticGroundWall{0, 200, 0, 255};     // darker green
    inline constexpr sf::Color kDebugStaticPlatformWall{0, 180, 180, 255}; // teal
    inline constexpr sf::Color kDebugStaticObstacleWall{200, 0, 200, 255}; // purple

} // namespace Palette
