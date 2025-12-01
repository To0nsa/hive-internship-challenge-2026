#pragma once

#include "collision/CollisionLayers.h"
#include "utils/Geom.h"
#include "utils/Math.h"

#include <SFML/System/Vector2.hpp>

class Entity;

// Basic damage payload passed from collision/system code to actors.
// Kept small on purpose; extend with knockback, damage types, etc. when needed.
struct DamageInfo {
    float          amount = 0.f;                       // raw HP loss for this hit
    sf::Vector2f   hitPoint{0.f, 0.f};                 // world-space point of impact (best-effort)
    sf::Vector2f   hitDir{0.f, 0.f};                   // normalized direction into the victim
    const Entity*  source      = nullptr;              // non-owning pointer to the damage source
    CollisionLayer collideWith = CollisionLayer::None; // layer of the source

    // Future extensions:
    // float      knockbackStrength = 0.f;       // scalar used by actors for physics response
    // float      bloodAmount       = 0.f;       // hints for VFX systems
    // DamageType type              = DamageType::Generic;
};

// Helper to build a basic DamageInfo from two AABBs and an optional damage source.
inline DamageInfo buildDamageInfo(float amount, const Entity* source,
                                  const sf::FloatRect& victimAabb,
                                  const sf::FloatRect* sourceAabb = nullptr) {
    DamageInfo info{};
    info.amount      = amount;
    info.source      = source;
    info.collideWith = source ? source->getCollisionLayer() : CollisionLayer::None;

    // Approximate hit point: center of overlap if possible, otherwise victim center.
    if (sourceAabb) {
        sf::FloatRect overlap;
        if (geom::aabbIntersects(*sourceAabb, victimAabb, overlap)) {
            info.hitPoint = {overlap.position.x + overlap.size.x * 0.5f,
                             overlap.position.y + overlap.size.y * 0.5f};
        } else {
            info.hitPoint = {victimAabb.position.x + victimAabb.size.x * 0.5f,
                             victimAabb.position.y + victimAabb.size.y * 0.5f};
        }
    } else {
        info.hitPoint = {victimAabb.position.x + victimAabb.size.x * 0.5f,
                         victimAabb.position.y + victimAabb.size.y * 0.5f};
    }

    // Hit direction points into the victim from the impact point (best-effort).
    const sf::Vector2f victimCenter{victimAabb.position.x + victimAabb.size.x * 0.5f,
                                    victimAabb.position.y + victimAabb.size.y * 0.5f};
    info.hitDir = math::normalizeVec(victimCenter - info.hitPoint);

    return info;
}
