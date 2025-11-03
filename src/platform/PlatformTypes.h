#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <cstdint>
#include <string>

// Static-image platforms you can stand on.
enum class PlatformKind : uint8_t { Platform1, Platform2, Count };

struct PlatformDesc {
    PlatformKind kind;
    const char*  textureKey; // key for ResourceManager
    sf::Vector2i frameSize{0, 0};
    sf::Vector2f visualOffset{0.f, 0.f};
    sf::Vector2f colliderSize{0.f, 0.f};
    sf::Vector2f scale{1.f, 1.f};
};

static const std::array<PlatformDesc, static_cast<size_t>(PlatformKind::Count)> kPlatformTable = {{
    PlatformDesc{PlatformKind::Platform1, "platform1.png",
                 /*frameSize*/ {64, 64},
                 /*visualOffset*/ {0.f, -180.f},
                 /*colliderSize*/ {24.f, 16.f},
                 /*scale*/ {2.f, 2.f}},
    PlatformDesc{PlatformKind::Platform2, "platform2.png",
                 /*frameSize*/ {64, 64},
                 /*visualOffset*/ {0.f, -150.f},
                 /*colliderSize*/ {24.f, 16.f},
                 /*scale*/ {2.f, 2.f}},
}};

inline const PlatformDesc& getPlatformDesc(PlatformKind k) {
    return kPlatformTable.at(static_cast<size_t>(k));
}
