#pragma once

#include "entities/platform/Platform.h"
#include "entities/platform/PlatformTypes.h"

class World;

// Inline spawn helper
inline Platform* spawnPlatform(World& world, PlatformKind kind, const sf::Vector2f& pos) {
    const PlatformDesc& desc = getPlatformDesc(kind);
    auto*               p    = world.createEntity<Platform>(desc);
    if (!p || !p->init())
        return nullptr;
    p->setPosition({pos + desc.visualOffset});
    p->update(0.f);
    return p;
}
