
#pragma once

#include "core/Assets.h"

#include <SFML/Graphics.hpp>
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

enum class ObstacleKind : uint8_t { Altar1, Altar2, Tree, Lich, SkullPile, Count };

struct ObstacleDesc {
    ObstacleKind     kind;
    bool             animated;
    std::string_view textureKey;

    // Animation source as a grid on a spritesheet
    sf::Vector2i frameSize{0, 0};
    sf::Vector2i startCell{0, 0};
    sf::Vector2i endCell{0, 0};
    float        fps  = 0.f;
    bool         loop = true;

    // Rendering + collision
    sf::Vector2f visualOffset{0.f, 0.f};
    sf::Vector2f colliderSize{0.f, 0.f};
    sf::Vector2f scale{1.f, 1.f};
    float        dps = 0.f;
};

static const std::array<ObstacleDesc, static_cast<size_t>(ObstacleKind::Count)> kTable = {{
    // Altar1
    ObstacleDesc{ObstacleKind::Altar1, true, Assets::Tex::Obstacle::Altar1,
                 /*frameSize*/ {80, 112}, /*start*/ {0, 0}, /*end*/ {5, 2},
                 /*fps*/ 8.f, /*loop*/ true,
                 /*visualOffset*/ {0.f, -120.f},
                 /*colliderSize*/ {30.f, 40.f},
                 /*scale*/ {4.f, 4.f},
                 /*dps*/ 10.f},
    // Altar2
    ObstacleDesc{ObstacleKind::Altar2, true, Assets::Tex::Obstacle::Altar2,
                 /*frameSize*/ {96, 112}, /*start*/ {0, 0}, /*end*/ {5, 2},
                 /*fps*/ 8.f, /*loop*/ true,
                 /*visualOffset*/ {0.f, -120.f},
                 /*colliderSize*/ {40.f, 50.f},
                 /*scale*/ {4.f, 4.f},
                 /*dps*/ 10.f},
    // Tree (static)
    ObstacleDesc{ObstacleKind::Tree, false, Assets::Tex::Obstacle::Tree,
                 /*frameSize*/ {128, 128}, /*start*/ {0, 0}, /*end*/ {0, 0},
                 /*fps*/ 0.f, /*loop*/ false,
                 /*visualOffset*/ {0.f, 20.f},
                 /*colliderSize*/ {20.f, 90.f},
                 /*scale*/ {2.f, 2.f},
                 /*dps*/ 10.f},
    // Lich statue (static)
    ObstacleDesc{ObstacleKind::Lich, false, Assets::Tex::Obstacle::Lich,
                 /*frameSize*/ {256, 256}, /*start*/ {0, 0}, /*end*/ {0, 0},
                 /*fps*/ 0.f, /*loop*/ false,
                 /*visualOffset*/ {0.f, 30.f},
                 /*colliderSize*/ {60.f, 80.f},
                 /*scale*/ {4.f, 4.f},
                 /*dps*/ 10.f},
    // Skull pile (static)
    ObstacleDesc{ObstacleKind::SkullPile, false, Assets::Tex::Obstacle::SkullPile,
                 /*frameSize*/ {128, 128}, /*start*/ {0, 0}, /*end*/ {0, 0},
                 /*fps*/ 0.f, /*loop*/ false,
                 /*visualOffset*/ {0.f, -40.f},
                 /*colliderSize*/ {60.f, 40.f},
                 /*scale*/ {4.f, 4.f},
                 /*dps*/ 10.f},
}};

inline const ObstacleDesc& getObstacleDesc(ObstacleKind kind) {
    return kTable.at(static_cast<size_t>(kind));
}
