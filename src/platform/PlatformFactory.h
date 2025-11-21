#pragma once

#include "Platform.h"
#include "PlatformTypes.h"

class World;

// Tiny, inline spawn helper consistent with spawnObstacle
inline Platform* spawnPlatform(World& world, PlatformKind kind, const sf::Vector2f& pos) {
    const PlatformDesc& desc = getPlatformDesc(kind);
    auto*               p    = world.createEntity<Platform>(desc);
    if (!p || !p->init())
        return nullptr;
    p->setPosition({pos + desc.visualOffset});
    p->update(0.f);
    return p;
}
