#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <cstdint>
#include <vector>

// Kind of static solid in the world: environment ground band, platforms, obstacles.
enum class StaticSolidKind : std::uint8_t {
    GroundBand,
    Platform,
    Obstacle,
};

// Which side(s) of a solid are collidable.
enum StaticSolidSide : std::uint8_t {
    SolidSide_None   = 0,
    SolidSide_Top    = 1u << 0,
    SolidSide_Bottom = 1u << 1,
    SolidSide_Left   = 1u << 2,
    SolidSide_Right  = 1u << 3,
};

inline StaticSolidSide operator|(StaticSolidSide a, StaticSolidSide b) {
    return static_cast<StaticSolidSide>(
        static_cast<std::uint8_t>(a) | static_cast<std::uint8_t>(b));
}

inline bool hasSide(std::uint8_t flags, StaticSolidSide side) {
    return (flags & static_cast<std::uint8_t>(side)) != 0;
}

// One static solid piece in the world.
struct StaticSolid {
    sf::FloatRect   rect;
    StaticSolidKind kind = StaticSolidKind::GroundBand;
    std::uint8_t    sides = static_cast<std::uint8_t>(StaticSolidSide::SolidSide_None);
};

// Aggregated static world geometry consumed by the physics system.
struct StaticWorldGeometry {
    std::vector<StaticSolid> solids;
};

