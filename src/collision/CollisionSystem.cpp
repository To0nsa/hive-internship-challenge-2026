#include "collision/CollisionSystem.h"

#include "collision/CollisionLayers.h"
#include "core/World.h"
#include "entities/actor/enemy/Enemy.h"
#include "entities/actor/player/Player.h"
#include "entities/collectible/RedSquare.h"
#include "entities/obstacle/Obstacle.h"
#include "entities/platform/Platform.h"
#include "environment/Environment.h"
#include "gameplay/Damage.h"
#include "gameplay/GameSession.h"
#include "spell/projectile/Projectile.h"
#include "utils/Geom.h"
#include "utils/Math.h"

namespace {
    void resolvePlayerObstacleContactDps(const CollisionContext& ctx) {
        Player* player = ctx.player;
        if (!player || !player->isAlive())
            return;

        const sf::FloatRect playerBounds = player->getCollider().worldAabb();
        for (auto* obstacle : ctx.obstacles) {
            if (!obstacle || !obstacle->isAlive())
                continue;

            const sf::FloatRect obstacleBounds = obstacle->getCollider().worldAabb();
            if (!geom::aabbIntersects(playerBounds, obstacleBounds))
                continue;

            // Contact damage when intersecting obstacle sides; separation is handled in physics.
            DamageInfo info = buildDamageInfo(obstacle->getDps() * ctx.dt, obstacle, playerBounds,
                                              &obstacleBounds);
            player->applyDamage(info);
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
            if (geom::touchTop(playerBounds, obstacleBounds, 0.75f)) {
                DamageInfo info = buildDamageInfo(obstacle->getDps() * ctx.dt, obstacle,
                                                  playerBounds, &obstacleBounds);
                player->applyDamage(info);
            }
        }
    }

    void resolveProjectileHits(const CollisionContext& ctx) {
        for (auto* proj : ctx.projectiles) {
            if (!proj || !proj->isAlive() || !proj->isDamageActive())
                continue;

            const sf::FloatRect  projectileBounds = proj->getCollider().worldAabb();
            const CollisionLayer projLayer        = proj->getCollisionLayer();
            const CollisionMask  projMask         = proj->getCollisionMask();

            if (projLayer == CollisionLayer::EnemyProjectile) {
                Player* player = ctx.player;
                if (!player || !player->isAlive())
                    continue;
                if (!layersCanCollide(projLayer, projMask, player->getCollisionLayer(),
                                      player->getCollisionMask()))
                    continue;
                const sf::FloatRect playerBounds = player->getCollider().worldAabb();
                if (geom::aabbIntersects(projectileBounds, playerBounds)) {
                    DamageInfo info = buildDamageInfo(proj->getStats().damage, proj, playerBounds,
                                                      &projectileBounds);
                    player->applyDamage(info);
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
                        DamageInfo  info     = buildDamageInfo(proj->getStats().damage, proj,
                                                               enemyBounds, &projectileBounds);
                        enemy->applyDamage(info);
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
                                  collectible->getCollisionLayer(),
                                  collectible->getCollisionMask()))
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
            if (ctx.environment->intersectsHazard(playerBounds, *ctx.cameraView)) {
                // Treat environment hazard as an instant-kill hit without a concrete source entity.
                DamageInfo info = buildDamageInfo(10000.f, nullptr, playerBounds, nullptr);
                player->applyDamage(info);
            }
        }

        if (geom::right(playerBounds) < ctx.cameraLeft) {
            DamageInfo info = buildDamageInfo(10000.f, nullptr, playerBounds, nullptr);
            player->applyDamage(info);
        }
    }

} // namespace

void collision::resolve(const CollisionContext& ctx) {
    resolvePlayerObstacleContactDps(ctx);
    resolveProjectileHits(ctx);
    resolveObstacleTopDps(ctx);
    resolvePlayerCollectibles(ctx);
    resolveHazards(ctx);
}

CollisionContext collision::buildCollisionContext(const World& world, float dt,
                                                  const MultiRectCollider* groundCollider) {
    CollisionContext ctx;
    ctx.dt          = dt;
    ctx.player      = world.m_pPlayer;
    ctx.ground      = groundCollider;
    ctx.cameraView  = &world.m_camera.getView();
    ctx.cameraLeft  = world.getCameraLeft();
    ctx.session     = &world.m_session;
    ctx.environment = &world.m_environment;

    ctx.actors.reserve(world.m_entities.size());
    ctx.enemies.reserve(world.m_entities.size());
    ctx.projectiles.reserve(world.m_entities.size());
    ctx.obstacles.reserve(world.m_entities.size());
    ctx.platforms.reserve(world.m_entities.size());
    ctx.collectibles.reserve(world.m_entities.size());

    for (auto& entity : world.m_entities) {
        if (!entity->isAlive())
            continue;

        switch (entity->getCollisionLayer()) {
        case CollisionLayer::Player:
            ctx.actors.push_back(static_cast<Actor*>(entity.get()));
            break;
        case CollisionLayer::Enemy:
            ctx.actors.push_back(static_cast<Actor*>(entity.get()));
            ctx.enemies.push_back(static_cast<Enemy*>(entity.get()));
            break;
        case CollisionLayer::PlayerProjectile:
        case CollisionLayer::EnemyProjectile:
            ctx.projectiles.push_back(static_cast<Projectile*>(entity.get()));
            break;
        case CollisionLayer::Obstacle:
            ctx.obstacles.push_back(static_cast<Obstacle*>(entity.get()));
            break;
        case CollisionLayer::Platform:
            ctx.platforms.push_back(static_cast<Platform*>(entity.get()));
            break;
        case CollisionLayer::Collectible:
            ctx.collectibles.push_back(static_cast<RedSquare*>(entity.get()));
            break;
        default:
            break;
        }
    }

    return ctx;
}
