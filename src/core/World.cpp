#include "core/World.h"

#include "collision/CollisionLayers.h"
#include "collision/CollisionSystem.h"
#include "collision/MultiRectCollider.h"
#include "core/Config.h"
#include "core/Debug.h"
#include "entities/actor/Actor.h"
#include "entities/actor/enemy/Demon.h"
#include "entities/actor/enemy/Enemy.h"
#include "entities/actor/enemy/FireWorm.h"
#include "entities/actor/player/Player.h"
#include "entities/collectible/RedSquare.h"
#include "entities/obstacle/Obstacle.h"
#include "entities/obstacle/ObstacleFactory.h"
#include "entities/obstacle/ObstacleTypes.h"
#include "entities/platform/Platform.h"
#include "entities/platform/PlatformFactory.h"
#include "spell/projectile/Projectile.h"
#include "states/StatePlaying.h"
#include "utils/Geom.h"
#include "utils/Random.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Mouse.hpp>
#include <algorithm>
#include <memory>
#include <vector>

World::World(StatePlaying& owner, GameSession& session, sf::RenderWindow& window)
    : m_owner(owner), m_session(session), m_window(window) {}

World::~World() = default;

bool World::init() {
    // Initialize camera with window size
    const sf::Vector2f viewSize{static_cast<float>(Config::windowWidth),
                                static_cast<float>(Config::windowHeight)};
    m_camera = Camera(viewSize);

    // Initialize environment
    if (!m_environment.initVolcanoDay())
        return false;
    m_environment.update(0.f, m_camera.getView());

    // Create player entity
    m_pPlayer = createEntity<Player>();
    if (!m_pPlayer || !m_pPlayer->init())
        return false;
    m_pPlayer->setPosition(sf::Vector2f(Config::windowWidth * 0.5f, Config::windowHeight * 0.5f));

    // Seed RNG and schedule first obstacle spawn a bit ahead of view
    // Random::seed(Random::timeSeed()); // uncomment for non-deterministic runs

    // Schedule first obstacle and platform
    const float viewRight = getCameraRight();
    m_nextObstacleX       = viewRight + Random::rangef(880.f, 1680.f);
    m_nextPlatformX       = viewRight + Random::rangef(680.f, 980.f);

    return true;
}

void World::update(float dt) {
    // Update all entities
    for (std::size_t i = 0; i < m_entities.size(); ++i) {
        Entity* e = m_entities[i].get();
        if (e && e->isAlive())
            e->update(dt);
    }

    // Update camera
    m_camera.update(dt, (m_pPlayer && m_pPlayer->isAlive()) ? m_pPlayer : nullptr);

    // Update environment and build ground collider
    m_environment.update(dt, m_camera.getView());
    const MultiRectCollider* groundCollider = m_environment.getGroundCollider();

    // Build combined static solids for physics (ground + obstacles + platforms).
    updateStaticSolidsCollider(groundCollider);

    // Step physics. At this stage, bodies may or may not be registered yet;
    // wiring happens in the actors that opt into physics.
    m_physics.step(dt, &m_staticSolids);

    // Resolve collisions
    CollisionContext collisionCtx = buildCollisionContext(dt, groundCollider);
    collision::resolve(collisionCtx);

    // TO-DO build enemy spawn system
    // Timed Demon spawns: every 10 seconds at y=400, just off the right edge of the view
    m_demonSpawnTimer -= dt;
    if (m_demonSpawnTimer <= 0.f) {
        m_demonSpawnTimer += 10.f;
        const float rightX       = getCameraRight() + 60.f;
        const float demonY       = 400.f;
        const float wormYDefault = 300.f;

        if (auto* demon = createEntity<Demon>()) {
            if (demon->init()) {
                demon->setPosition({rightX, demonY});
                demon->update(0.f);
            }
        }

        // Spawn FireWorm aligned to safe ground if available at this X; otherwise fall back
        // to a default height.
        GroundSample wormSample = sampleGround(rightX, 64.f);
        if (auto* worm = createEntity<FireWorm>()) {
            if (worm->init()) {
                const bool useGround = wormSample.isSafe();
                float      spawnY    = useGround ? wormSample.topY : wormYDefault;

                worm->setPosition({rightX, spawnY});

                if (useGround) {
                    const sf::FloatRect aabb   = worm->getCollider().worldAabb();
                    const float         bottom = aabb.position.y + aabb.size.y;
                    const float         dy     = wormSample.topY - bottom;
                    auto                pos    = worm->getPosition();
                    pos.y += dy;
                    worm->setPosition(pos);
                }

                worm->update(0.f);
            }
        }
    }

    // TO-DO build level generation system
    // Stream simple random obstacles ahead of camera
    {
        const float viewRight = m_camera.right();
        if (groundCollider) {
            while (viewRight + 50.f >= m_nextObstacleX) {
                const int   kindIdx = Random::rangei(0, static_cast<int>(ObstacleKind::Count) - 1);
                const auto  kind    = static_cast<ObstacleKind>(kindIdx);
                const auto& desc    = getObstacleDesc(kind);
                const float x       = m_nextObstacleX;

                GroundSample sample = sampleGround(x, desc.colliderSize.x);
                if (sample.isSafe()) {
                    const float y = sample.topY - desc.colliderSize.y;
                    spawnObstacle(*this, kind, {x, y});
                }

                m_nextObstacleX += Random::rangef(680.f, 1280.f);
            }
        }
    }
    // Stream simple random platforms ahead of camera
    {
        const float viewRight = m_camera.right();
        if (groundCollider) {
            const float groundTop = m_environment.getGroundTopY(m_camera.getView());
            while (viewRight + 50.f >= m_nextPlatformX) {
                const int   kindIdx = Random::rangei(0, static_cast<int>(PlatformKind::Count) - 1);
                const auto  kind    = static_cast<PlatformKind>(kindIdx);
                const auto& desc    = getPlatformDesc(kind);
                const float y = groundTop - desc.colliderSize.y - Random::rangef(100.f, 300.f);
                const float x = m_nextPlatformX;
                if (auto* p = spawnPlatform(*this, kind, {x, y})) {
                    // Spawn a red square 10px above the platform top, centered horizontally
                    const sf::FloatRect pb  = p->getCollider().worldAabb();
                    const float         top = pb.position.y;
                    const float         cx  = pb.position.x + pb.size.x * 0.5f;
                    const float         cy  = top - 10.f - (RedSquare::kSize * 0.5f);
                    if (auto* sq = createEntity<RedSquare>(sf::Vector2f{cx, cy}))
                        (void)sq->init();
                }
                m_nextPlatformX += Random::rangef(480.f, 880.f);
            }
        }
    }

    // Lifetime culling, mark obstacles, platforms, collectibles dead if offscreen
    cullOffscreen();

    // Remove dead entities and unregister any physics bodies they owned.
    m_entities.erase(std::remove_if(m_entities.begin(), m_entities.end(),
                                    [this](const auto& e) {
                                        if (!e->isAlive()) {
                                            m_physics.unregisterBody(*e);
                                            return true;
                                        }
                                        return false;
                                    }),
                     m_entities.end());
}

void World::render(sf::RenderTarget& target) const {
    const sf::View oldView = target.getView();
    m_camera.apply(target);

    m_environment.renderBackground(target, m_camera.getView());

    for (const std::unique_ptr<Entity>& pEntity : m_entities) {
        if (dynamic_cast<Actor*>(pEntity.get()))
            pEntity->render(target);
    }

    m_environment.renderForeground(target, m_camera.getView());

    for (const std::unique_ptr<Entity>& pEntity : m_entities) {
        if (!dynamic_cast<Actor*>(pEntity.get()))
            pEntity->render(target);
    }

    // Debug helpers
    if constexpr (Config::kDebugDraw) {
        Debug::drawCameraGuides(target, m_camera.getView(), getCameraCatchupX(),
                                getFollowThresholdX());

        if (const MultiRectCollider* ground = m_environment.getGroundCollider()) {
            const auto& rects = ground->getRectColliders();
            for (const auto& r : rects) {
                Debug::drawRectOutline(target, r, sf::Color::Green, 1.f);
            }
        }

        Debug::drawGroundSampleBands(
            target, m_camera.getView(),
            [&env = m_environment, view = m_camera.getView()](float x, float w) {
                return sampleGroundFromEnvironment(env, view, x, w);
            });
    }

    target.setView(oldView);
}

float World::getCameraLeft() const { return m_camera.left(); }

float World::getCameraRight() const { return m_camera.right(); }

float World::getCameraCatchupX() const { return m_camera.catchupX(); }

float World::getFollowThresholdX() const { return m_camera.followThresholdX(); }

sf::Vector2f World::getMouseWorld() const {
    const sf::Vector2i pixel = sf::Mouse::getPosition(m_window);
    return m_camera.mapPixelToWorld(m_window, pixel);
}

GroundSample World::sampleGround(float x, float width) const {
    return sampleGroundFromEnvironment(m_environment, m_camera.getView(), x, width);
}

bool World::canSpawnOnGroundAt(float x, float width) const {
    const GroundSample sample = sampleGround(x, width);
    return sample.isSafe();
}

void World::addScore(int points) { m_session.addScore(points); }

void World::requestExitToMenu() { m_owner.requestExitToMenu(); }

CollisionContext World::buildCollisionContext(float dt, const MultiRectCollider* groundCollider) {
    CollisionContext ctx;
    ctx.dt          = dt;
    ctx.player      = m_pPlayer;
    ctx.ground      = groundCollider;
    ctx.cameraView  = &m_camera.getView();
    ctx.cameraLeft  = getCameraLeft();
    ctx.session     = &m_session;
    ctx.environment = &m_environment;

    ctx.actors.reserve(m_entities.size());
    ctx.enemies.reserve(m_entities.size());
    ctx.projectiles.reserve(m_entities.size());
    ctx.obstacles.reserve(m_entities.size());
    ctx.platforms.reserve(m_entities.size());
    ctx.collectibles.reserve(m_entities.size());

    for (auto& entity : m_entities) {
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

void World::cullOffscreen() {
    const float viewLeft   = getCameraLeft();
    const float cullBefore = viewLeft - m_camera.getView().getSize().x;

    for (auto& entity : m_entities) {
        if (!entity->isAlive())
            continue;

        const CollisionLayer layer = entity->getCollisionLayer();
        if (layer != CollisionLayer::Obstacle && layer != CollisionLayer::Platform &&
            layer != CollisionLayer::Collectible)
            continue;

        const sf::FloatRect aabb  = entity->getCollider().worldAabb();
        const float         right = aabb.position.x + aabb.size.x;
        if (right < cullBefore)
            entity->setAlive(false);
    }
}

void World::updateStaticSolidsCollider(const MultiRectCollider* groundCollider) {
    std::vector<sf::FloatRect> solids;
    if (groundCollider) {
        solids = groundCollider->getRectColliders();
    }
    solids.reserve(solids.size() + m_entities.size());

    for (const auto& entity : m_entities) {
        if (!entity->isAlive())
            continue;

        const CollisionLayer layer = entity->getCollisionLayer();
        if (layer != CollisionLayer::Obstacle && layer != CollisionLayer::Platform)
            continue;

        solids.push_back(entity->getCollider().worldAabb());
    }

    m_staticSolids.setRectColliders(std::move(solids));
}
