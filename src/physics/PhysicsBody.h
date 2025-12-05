#pragma once

#include <SFML/System/Vector2.hpp>

class Entity;
class MultiRectCollider;

// Configuration for how a body participates in physics.
struct PhysicsBodyConfig {
    bool enabled       = true;
    bool isKinematic   = false; // if true, physics does not integrate position/velocity
    bool useGravity    = true;
    bool topOnlyGround = true; // if true, only resolve top contacts (platformer-style)

    // Scale applied to the global gravity configured on PhysicsSystem.
    // 1.0 = normal gravity, 0.0 = no gravity, >1.0 = stronger gravity.
    float gravityScale = 1.f;

    float maxVelX = 3000.f;
    float maxVelY = 3000.f;
};

// Per-entity physics state owned by PhysicsSystem.
struct PhysicsBody {
    Entity*           owner = nullptr; // non-owning
    PhysicsBodyConfig config;

    bool grounded = false;
};
