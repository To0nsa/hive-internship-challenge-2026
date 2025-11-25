#pragma once

#include <cstdint>
#include <initializer_list>

// Basic collision layer + mask helpers. Layers are mutually exclusive per entity;
// masks express which layers this entity is willing to interact with.
enum class CollisionLayer : std::uint8_t {
    None = 0,
    Player,
    Enemy,
    PlayerProjectile,
    EnemyProjectile,
    Obstacle,
    Platform,
    Collectible,
    Count
};

using CollisionMask = std::uint32_t;

constexpr CollisionMask maskNone() { return 0u; }
constexpr CollisionMask maskAll() { return 0xFFFFFFFFu; }

constexpr CollisionMask maskFor(CollisionLayer layer) {
    return static_cast<CollisionMask>(1u) << static_cast<std::uint8_t>(layer);
}

constexpr CollisionMask maskFrom(std::initializer_list<CollisionLayer> layers) {
    CollisionMask mask = maskNone();
    for (auto layer : layers)
        mask |= maskFor(layer);
    return mask;
}

constexpr bool maskContains(CollisionMask mask, CollisionLayer layer) {
    return (mask & maskFor(layer)) != 0;
}

// Two entities "can collide" only if both masks agree on each other's layer.
constexpr bool layersCanCollide(CollisionLayer aLayer, CollisionMask aMask, CollisionLayer bLayer,
                                CollisionMask bMask) {
    return maskContains(aMask, bLayer) && maskContains(bMask, aLayer);
}
