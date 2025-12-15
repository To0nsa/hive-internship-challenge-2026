#pragma once

#include "physics/StaticWorldGeometry.h"

#include <memory>
#include <vector>

class Entity;
class MultiRectCollider;

namespace physics {
    StaticWorldGeometry
    buildStaticWorldGeometry(const MultiRectCollider*                            groundCollider,
                             const std::vector<std::unique_ptr<Entity>>& entities);
} // namespace physics

