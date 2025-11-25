#pragma once

#include "collision/CollisionLayers.h"
#include "entities/actor/Actor.h"

class Enemy : public Actor {
  public:
    Enemy() {
        setCollisionLayer(CollisionLayer::Enemy);
        setCollisionMask(maskFrom({CollisionLayer::PlayerProjectile}));
    }
    ~Enemy() override = default;
};
