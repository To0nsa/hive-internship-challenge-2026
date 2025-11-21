#pragma once

#include "Obstacle.h"
#include "ObstacleTypes.h"

class World;

// Tiny, inline spawn helper that hides the registry lookup
inline Obstacle* spawnObstacle(World& world, ObstacleKind kind, const sf::Vector2f& pos) {
    const ObstacleDesc& desc = getObstacleDesc(kind);
    auto*               obs  = world.createEntity<Obstacle>(desc);
    if (!obs || !obs->init())
        return nullptr;
    // Position sprite so its visual top-left aligns at pos + visualOffset
    obs->setPosition({pos + desc.visualOffset});
    obs->update(0.f);
    return obs;
}
