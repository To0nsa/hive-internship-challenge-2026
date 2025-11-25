#include "collision/CollisionSystem.h"

#include "collision/CollisionLayers.h"
#include "entities/actor/Actor.h"
#include "entities/actor/enemy/Enemy.h"
#include "entities/actor/player/Player.h"
#include "entities/collectible/RedSquare.h"
#include "entities/obstacle/Obstacle.h"
#include "entities/platform/Platform.h"
#include "environment/Environment.h"
#include "gameplay/GameSession.h"
#include "spell/projectile/Projectile.h"
#include "utils/Geom.h"

namespace {

    MultiRectCollider buildSolidCollider(const CollisionContext& ctx) {
        MultiRectCollider combined;

        std::vector<sf::FloatRect> solids =
            ctx.ground ? ctx.ground->getRectColliders() : std::vector<sf::FloatRect>{};
        solids.reserve(solids.size() + ctx.obstacles.size() + ctx.platforms.size());

        for (auto* obstacle : ctx.obstacles) {
            if (obstacle && obstacle->isAlive())
                solids.push_back(obstacle->getCollider().worldAabb());
        }
        for (auto* platform : ctx.platforms) {
            if (platform && platform->isAlive())
                solids.push_back(platform->getCollider().worldAabb());
        }

        combined.setRectColliders(std::move(solids));
        return combined;
    }

    void applyActorGroundContacts(const CollisionContext& ctx, const MultiRectCollider& combined) {
        const Collider* ground = combined.getRectColliders().empty()
                                     ? static_cast<const Collider*>(nullptr)
                                     : static_cast<const Collider*>(&combined);

        for (auto* actor : ctx.actors) {
            if (!actor || !actor->isAlive())
                continue;
            actor->applyPhysics(ctx.dt, ground);
        }
    }

    void resolvePlayerObstacleSides(const CollisionContext& ctx) {
        Player* player = ctx.player;
        if (!player || !player->isAlive())
            return;

        // Current player bounds; refreshed after each push to avoid tunneling.
        sf::FloatRect playerBounds = player->getCollider().worldAabb();
        for (auto* obstacle : ctx.obstacles) {
            if (!obstacle || !obstacle->isAlive())
                continue;

            const sf::FloatRect obstacleBounds = obstacle->getCollider().worldAabb();
            sf::FloatRect       overlap;
            if (!geom::aabbIntersects(playerBounds, obstacleBounds, overlap))
                continue;

            // Contact = damage + horizontal separation (no vertical response here).
            player->applyDamage(obstacle->getDps() * ctx.dt);

            const float playerCenterX   = playerBounds.position.x + playerBounds.size.x * 0.5f;
            const float obstacleCenterX = obstacleBounds.position.x + obstacleBounds.size.x * 0.5f;
            const float pushX =
                (playerCenterX < obstacleCenterX) ? -overlap.size.x : overlap.size.x;
            const auto pos = player->getPosition();
            player->setPosition({pos.x + pushX, pos.y});

            playerBounds = player->getCollider().worldAabb();
        }
    }

    void resolveObstacleTopDps(const CollisionContext& ctx) {
        Player* player = ctx.player;
        if (!player || !player->isAlive())
            return;

        const sf::FloatRect playerBounds = player->getCollider().worldAabb();
        for (auto* obstacle : ctx.obstacles) {
            if (!obstacle || !obstacle->isAlive())
                continue;
            const sf::FloatRect obstacleBounds = obstacle->getCollider().worldAabb();
            if (geom::touchTop(playerBounds, obstacleBounds, 0.75f))
                player->applyDamage(obstacle->getDps() * ctx.dt);
        }
    }

    void resolveProjectileHits(const CollisionContext& ctx) {
        for (auto* proj : ctx.projectiles) {
            if (!proj || !proj->isAlive() || !proj->isDamageActive())
                continue;

            const sf::FloatRect  projectileBounds = proj->getCollider().worldAabb();
            const CollisionLayer projLayer = proj->getCollisionLayer();
            const CollisionMask  projMask  = proj->getCollisionMask();

            if (projLayer == CollisionLayer::EnemyProjectile) {
                Player* player = ctx.player;
                if (!player || !player->isAlive())
                    continue;
                if (!layersCanCollide(projLayer, projMask, player->getCollisionLayer(),
                                      player->getCollisionMask()))
                    continue;
                const sf::FloatRect playerBounds = player->getCollider().worldAabb();
                if (geom::aabbIntersects(projectileBounds, playerBounds)) {
                    player->applyDamage(proj->getStats().damage);
                    proj->requestImpact();
                }
            } else if (projLayer == CollisionLayer::PlayerProjectile) {
                bool hitEnemy = false;

                // Enemies
                for (auto* enemy : ctx.enemies) {
                    if (!enemy || !enemy->isAlive())
                        continue;
                    if (!layersCanCollide(projLayer, projMask, enemy->getCollisionLayer(),
                                          enemy->getCollisionMask()))
                        continue;
                    const sf::FloatRect enemyBounds = enemy->getCollider().worldAabb();
                    if (geom::aabbIntersects(projectileBounds, enemyBounds)) {
                        const float hpBefore = enemy->getHp();
                        enemy->applyDamage(proj->getStats().damage);
                        if (hpBefore > 0.f && enemy->getHp() <= 0.f && ctx.session)
                            ctx.session->addScore(100);
                        proj->requestImpact();
                        hitEnemy = true;
                        break;
                    }
                }
                if (hitEnemy)
                    continue;

                // Collectibles
                for (auto* collectible : ctx.collectibles) {
                    if (!collectible || !collectible->isAlive())
                        continue;
                    if (!layersCanCollide(projLayer, projMask, collectible->getCollisionLayer(),
                                          collectible->getCollisionMask()))
                        continue;
                    const sf::FloatRect collectibleBounds = collectible->getCollider().worldAabb();
                    if (geom::aabbIntersects(projectileBounds, collectibleBounds)) {
                        if (ctx.session)
                            ctx.session->addScore(100);
                        collectible->setAlive(false);
                        proj->requestImpact();
                        break;
                    }
                }
            }
        }
    }

    void resolvePlayerCollectibles(const CollisionContext& ctx) {
        Player* player = ctx.player;
        if (!player || !player->isAlive())
            return;

        const sf::FloatRect playerBounds = player->getCollider().worldAabb();
        for (auto* collectible : ctx.collectibles) {
            if (!collectible || !collectible->isAlive())
                continue;
            if (!layersCanCollide(player->getCollisionLayer(), player->getCollisionMask(),
                                  collectible->getCollisionLayer(), collectible->getCollisionMask()))
                continue;
            const sf::FloatRect collectibleBounds = collectible->getCollider().worldAabb();
            if (geom::aabbIntersects(playerBounds, collectibleBounds)) {
                if (ctx.session)
                    ctx.session->addScore(100);
                collectible->setAlive(false);
            }
        }
    }

    void resolveHazards(const CollisionContext& ctx) {
        Player* player = ctx.player;
        if (!player || !player->isAlive())
            return;

        const sf::FloatRect playerBounds = player->getCollider().worldAabb();
        if (ctx.environment && ctx.cameraView) {
            if (ctx.environment->intersectsLavaGap(playerBounds, *ctx.cameraView))
                player->applyDamage(10000.f);
        }

        if (geom::right(playerBounds) < ctx.cameraLeft)
            player->applyDamage(10000.f);
    }

} // namespace

void collision::resolve(const CollisionContext& ctx) {
    const MultiRectCollider combined = buildSolidCollider(ctx);

    applyActorGroundContacts(ctx, combined);
    resolvePlayerObstacleSides(ctx);
    resolveProjectileHits(ctx);
    resolveObstacleTopDps(ctx);
    resolvePlayerCollectibles(ctx);
    resolveHazards(ctx);
}
