#include "physics/StaticWorldGeometryBuilder.h"

#include "collision/CollisionLayers.h"
#include "collision/MultiRectCollider.h"
#include "entities/Entity.h"

namespace physics {
    StaticWorldGeometry
    buildStaticWorldGeometry(const MultiRectCollider* groundCollider,
                             const std::vector<std::unique_ptr<Entity>>& entities) {
        StaticWorldGeometry geometry;
        auto&               solids = geometry.solids;

        constexpr float kSideThickness = 4.f;

        const auto appendTopAndWalls = [&](const sf::FloatRect& rect, StaticSolidKind kind,
                                           bool fullTopRect) {
            const float left   = rect.position.x;
            const float top    = rect.position.y;
            const float width  = rect.size.x;
            const float height = rect.size.y;

            // Top surface: used for grounding. For ground band we keep the full rect (top-only)
            // since vertical resolution only cares about `top()`, but other solids can use a thin
            // strip.
            solids.push_back(StaticSolid{
                fullTopRect ? rect : sf::FloatRect{{left, top}, {width, kSideThickness}},
                kind,
                static_cast<std::uint8_t>(StaticSolidSide::SolidSide_Top),
            });

            // Left wall.
            solids.push_back(StaticSolid{
                sf::FloatRect{{left, top}, {kSideThickness, height}},
                kind,
                static_cast<std::uint8_t>(StaticSolidSide::SolidSide_Left),
            });

            // Right wall.
            solids.push_back(StaticSolid{
                sf::FloatRect{{left + width - kSideThickness, top}, {kSideThickness, height}},
                kind,
                static_cast<std::uint8_t>(StaticSolidSide::SolidSide_Right),
            });
        };

        // 1) Ground band -> top-only surfaces + walls for the physics side-collisions.
        if (groundCollider) {
            const auto& colliderRects = groundCollider->getRectColliders();
            solids.reserve(solids.size() + (colliderRects.size() * 3u));
            for (const auto& groundRect : colliderRects)
                appendTopAndWalls(groundRect, StaticSolidKind::GroundBand, /*fullTopRect=*/true);
        }

        // 2) Platforms and obstacles -> split into top + walls.
        solids.reserve(solids.size() + (entities.size() * 3u));
        for (const auto& entity : entities) {
            if (!entity || !entity->isAlive())
                continue;

            const CollisionLayer layer = entity->getCollisionLayer();
            if (layer != CollisionLayer::Obstacle && layer != CollisionLayer::Platform)
                continue;

            const sf::FloatRect bounds = entity->getCollider().worldAabb();
            StaticSolidKind kind = StaticSolidKind::Platform;
            if (layer == CollisionLayer::Obstacle)
                kind = StaticSolidKind::Obstacle;
            appendTopAndWalls(bounds, kind, /*fullTopRect=*/false);
        }

        return geometry;
    }
} // namespace physics
