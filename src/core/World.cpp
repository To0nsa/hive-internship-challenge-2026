#include "core/World.h"

#include "collision/MultiRectCollider.h"
#include "core/Config.h"
#include "core/Debug.h"
#include "entities/actor/Actor.h"
#include "entities/actor/enemy/Demon.h"
#include "entities/actor/enemy/Enemy.h"
#include "entities/actor/player/Player.h"
#include "entities/collectible/RedSquare.h"
#include "entities/obstacle/Obstacle.h"
#include "entities/obstacle/ObstacleFactory.h"
#include "entities/obstacle/ObstacleTypes.h"
#include "entities/platform/Platform.h"
#include "entities/platform/PlatformFactory.h"
#include "gameplay/Faction.h"
#include "spell/projectile/Projectile.h"
#include "states/StatePlaying.h"
#include "utils/Geom.h"
#include "utils/Math.h"
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
    // View set to window size
    m_view.setSize(
        {static_cast<float>(Config::windowWidth), static_cast<float>(Config::windowHeight)});
    m_view.setCenter({m_view.getSize().x * 0.5f, m_view.getSize().y * 0.5f});
    // Camera state
    m_cameraX       = m_view.getCenter().x;
    m_cameraTargetX = m_cameraX;
    m_cameraSpeed   = 0.f; // ease-in

    if (!m_environment.initVolcanoDay())
        return false;
    m_environment.update(0.f, m_view);

    // Create player entity
    m_pPlayer = createEntity<Player>();
    if (!m_pPlayer || !m_pPlayer->init())
        return false;
    m_pPlayer->setPosition(sf::Vector2f(Config::windowWidth * 0.5f, Config::windowHeight * 0.5f));

    // Seed RNG and schedule first obstacle spawn a bit ahead of view
    // Random::seed(Random::timeSeed()); // uncomment for non-deterministic runs

    const float viewRight = getCameraLeft() + m_view.getSize().x * 2.f;
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

    // CAMERA UPDATE
    // Camera auto-scroll baseline (advance target; center eases toward it)
    if (m_cameraSpeed < m_cameraTargetSpeed) {
        m_cameraSpeed = std::min(m_cameraTargetSpeed, m_cameraSpeed + kCameraAccel * dt);
    } else if (m_cameraSpeed > m_cameraTargetSpeed) {
        m_cameraSpeed = std::max(m_cameraTargetSpeed, m_cameraSpeed - kCameraAccel * dt);
    }
    m_cameraTargetX += m_cameraSpeed * dt;
    // If player passes the follow threshold, let the target drift toward player smoothly.
    if (m_pPlayer && m_pPlayer->isAlive()) {
        const float playerX         = m_pPlayer->getPosition().x;
        const float followThreshold = getFollowThresholdX();
        if (playerX > followThreshold) {
            const float alphaT    = math::expSmoothingFactor(kTargetCatchupLerp, dt);
            const float desired   = playerX; // player centered
            const float newTarget = m_cameraTargetX + (desired - m_cameraTargetX) * alphaT;
            m_cameraTargetX       = std::max(m_cameraTargetX, newTarget);
        }
    }
    // Ease camera center toward target
    const float alpha = math::expSmoothingFactor(kCatchupLerp, dt);
    m_cameraX += (m_cameraTargetX - m_cameraX) * alpha;
    m_view.setCenter({m_cameraX, m_view.getSize().y * 0.5f});

    m_environment.update(dt, m_view);
    const MultiRectCollider* groundCollider = m_environment.getGroundCollider();

    // Timed Demon spawns: every 10 seconds at y=400, just off the right edge of the view
    m_demonSpawnTimer -= dt;
    if (m_demonSpawnTimer <= 0.f) {
        m_demonSpawnTimer += 10.f;
        const float rightX = getCameraLeft() + m_view.getSize().x + 60.f;
        const float y      = 400.f;
        if (auto* demon = createEntity<Demon>()) {
            if (demon->init()) {
                demon->setPosition({rightX, y});
                demon->update(0.f);
            }
        }
    }

    // Resolve horizontal collisions between player and obstacles
    if (m_pPlayer && m_pPlayer->isAlive()) {
        sf::FloatRect pb = m_pPlayer->getCollider().worldAabb();
        for (auto& entity : m_entities) {
            if (!entity->isAlive())
                continue;
            if (auto* o = dynamic_cast<Obstacle*>(entity.get())) {
                const sf::FloatRect ob = o->getCollider().worldAabb();
                sf::FloatRect       inter;
                if (geom::aabbIntersects(pb, ob, inter)) {
                    if (m_pPlayer)
                        m_pPlayer->applyDamage(o->getDps() * dt);
                    const float pcx   = pb.position.x + pb.size.x * 0.5f;
                    const float ocx   = ob.position.x + ob.size.x * 0.5f;
                    const float pushX = (pcx < ocx ? -inter.size.x : +inter.size.x);
                    const auto  p     = m_pPlayer->getPosition();
                    m_pPlayer->setPosition({p.x + pushX, p.y});
                    pb = m_pPlayer->getCollider().worldAabb();
                }
            }
        }
    }

    // Apply physics to actors using combined ground + obstacle colliders (walk on them)
    MultiRectCollider combined;
    if (groundCollider) {
        std::vector<sf::FloatRect> solids = groundCollider->getRectColliders();
        solids.reserve(solids.size() + m_entities.size());
        for (auto& entity : m_entities) {
            if (!entity->isAlive())
                continue;
            if (auto* o = dynamic_cast<Obstacle*>(entity.get())) {
                solids.emplace_back(o->getCollider().worldAabb());
            } else if (auto* p = dynamic_cast<Platform*>(entity.get())) {
                solids.emplace_back(p->getCollider().worldAabb());
            }
        }
        combined.setRectColliders(std::move(solids));
    }
    for (auto& entity : m_entities) {
        if (!entity->isAlive())
            continue;
        if (auto* actor = dynamic_cast<Actor*>(entity.get())) {
            const Collider* col = groundCollider ? static_cast<const Collider*>(&combined)
                                                 : static_cast<const Collider*>(nullptr);
            actor->applyPhysics(dt, col);
        }
    }

    // Spell collisions: projectiles vs actors by faction
    {
        for (const auto& eProj : m_entities) {
            auto* proj = dynamic_cast<Projectile*>(eProj.get());
            if (!proj || !proj->isAlive() || !proj->isDamageActive())
                continue;

            const sf::FloatRect pr = proj->getCollider().worldAabb();
            if (proj->getFaction() == Faction::Enemy) {
                // Enemy projectile hits the player
                if (m_pPlayer && m_pPlayer->isAlive()) {
                    const sf::FloatRect pb = m_pPlayer->getCollider().worldAabb();
                    if (geom::aabbIntersects(pr, pb)) {
                        m_pPlayer->applyDamage(proj->getStats().damage);
                        proj->requestImpact();
                        continue;
                    }
                }
            } else if (proj->getFaction() == Faction::Player) {
                // Player projectile hits enemies or collectibles
                for (const auto& eOther : m_entities) {
                    // Enemies
                    if (auto* enemy = dynamic_cast<Enemy*>(eOther.get())) {
                        if (!enemy->isAlive())
                            continue;
                        const sf::FloatRect eb = enemy->getCollider().worldAabb();
                        if (geom::aabbIntersects(pr, eb)) {
                            // Award kill if this hit is lethal
                            const float hpBefore = enemy->getHp();
                            enemy->applyDamage(proj->getStats().damage);
                            if (hpBefore > 0.f && enemy->getHp() <= 0.f)
                                m_session.addScore(100); // enemy killed
                            proj->requestImpact();
                            break;
                        }
                        continue;
                    }
                    // RedSquare collectibles
                    if (auto* sq = dynamic_cast<RedSquare*>(eOther.get())) {
                        if (!sq->isAlive())
                            continue;
                        const sf::FloatRect qb = sq->getCollider().worldAabb();
                        if (geom::aabbIntersects(pr, qb)) {
                            addScore(100);
                            sq->setAlive(false);
                            proj->requestImpact();
                            break;
                        }
                    }
                }
            }
        }
    }

    // Apply DPS when standing on top of an obstacle
    if (m_pPlayer && m_pPlayer->isAlive()) {
        const sf::FloatRect pb = m_pPlayer->getCollider().worldAabb();
        for (auto& entity : m_entities) {
            if (!entity->isAlive())
                continue;
            if (auto* o = dynamic_cast<Obstacle*>(entity.get())) {
                const sf::FloatRect ob = o->getCollider().worldAabb();
                if (geom::touchTop(pb, ob, 0.75f)) {
                    m_pPlayer->applyDamage(o->getDps() * dt);
                }
            }
        }
    }

    // Kill if player falls into a lava gap
    if (m_pPlayer && m_pPlayer->isAlive()) {
        const sf::FloatRect pb = m_pPlayer->getCollider().worldAabb();
        if (m_environment.intersectsLavaGap(pb, m_view)) {
            m_pPlayer->applyDamage(10000.f);
        }
    }

    // Kill if the camera (left edge) catches up to the player
    if (m_pPlayer && m_pPlayer->isAlive()) {
        const sf::FloatRect pb      = m_pPlayer->getCollider().worldAabb();
        const float         camLeft = getCameraLeft();
        if (geom::right(pb) < camLeft) {
            m_pPlayer->applyDamage(10000.f);
        }
    }

    // Stream simple random obstacles ahead of camera
    {
        const float viewLeft  = getCameraLeft();
        const float viewRight = viewLeft + m_view.getSize().x * 2.f;
        if (groundCollider) {
            const float groundTop = m_environment.getGroundTopY(m_view);
            while (viewRight + 50.f >= m_nextObstacleX) {
                const int   kindIdx = Random::rangei(0, static_cast<int>(ObstacleKind::Count) - 1);
                const auto  kind    = static_cast<ObstacleKind>(kindIdx);
                const auto& desc    = getObstacleDesc(kind);
                const float y       = groundTop - desc.colliderSize.y;
                const float x       = m_nextObstacleX;
                spawnObstacle(*this, kind, {x, y});
                m_nextObstacleX += Random::rangef(680.f, 1280.f);
            }
        }
    }

    // Stream simple random platforms ahead of camera
    {
        const float viewLeft  = getCameraLeft();
        const float viewRight = viewLeft + m_view.getSize().x * 2.f;
        if (groundCollider) {
            const float groundTop = m_environment.getGroundTopY(m_view);
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

    // Lifetime culling: remove platforms behind the camera
    {
        const float viewLeft   = getCameraLeft();
        const float cullBefore = viewLeft - m_view.getSize().x;

        for (auto& entity : m_entities) {
            if (!entity->isAlive())
                continue;
            if (auto* p = dynamic_cast<Platform*>(entity.get())) {
                const sf::FloatRect aabb  = p->getCollider().worldAabb();
                const float         right = aabb.position.x + aabb.size.x;
                if (right < cullBefore)
                    p->setAlive(false);
            }
        }

        m_entities.erase(std::remove_if(m_entities.begin(), m_entities.end(),
                                        [](auto& e) { return !e->isAlive(); }),
                         m_entities.end());
    }

    // Lifetime culling: remove obstacles behind the camera
    {
        const float viewLeft   = getCameraLeft();
        const float cullBefore = viewLeft - m_view.getSize().x;

        for (auto& entity : m_entities) {
            if (!entity->isAlive())
                continue;
            if (auto* o = dynamic_cast<Obstacle*>(entity.get())) {
                const sf::FloatRect aabb  = o->getCollider().worldAabb();
                const float         right = aabb.position.x + aabb.size.x;
                if (right < cullBefore)
                    o->setAlive(false);
            }
        }

        m_entities.erase(std::remove_if(m_entities.begin(), m_entities.end(),
                                        [](auto& e) { return !e->isAlive(); }),
                         m_entities.end());
    }
}

const std::vector<sf::FloatRect>& World::getGroundRects() const {
    if (const auto* collider = m_environment.getGroundCollider())
        return collider->getRectColliders();
    static const std::vector<sf::FloatRect> kEmpty;
    return kEmpty;
}

const std::vector<sf::FloatRect>& World::getObstacleRects() const {
    m_cachedObstacleRects.clear();
    m_cachedObstacleRects.reserve(m_entities.size());
    for (const auto& entity : m_entities) {
        if (!entity->isAlive())
            continue;
        if (dynamic_cast<const Obstacle*>(entity.get())) {
            m_cachedObstacleRects.push_back(entity->getCollider().worldAabb());
        }
    }
    return m_cachedObstacleRects;
}

const std::vector<sf::FloatRect>& World::getPlatformRects() const {
    m_cachedPlatformRects.clear();
    m_cachedPlatformRects.reserve(m_entities.size());
    for (const auto& entity : m_entities) {
        if (!entity->isAlive())
            continue;
        if (dynamic_cast<const Platform*>(entity.get())) {
            m_cachedPlatformRects.push_back(entity->getCollider().worldAabb());
        }
    }
    return m_cachedPlatformRects;
}

const std::vector<sf::FloatRect>& World::getSolidTopRects() const {
    m_cachedSolidTopRects.clear();
    // Start with ground rects
    const auto& groundRects = getGroundRects();
    m_cachedSolidTopRects.reserve(groundRects.size() + m_entities.size());
    m_cachedSolidTopRects.insert(m_cachedSolidTopRects.end(), groundRects.begin(),
                                 groundRects.end());
    // Append obstacle rects
    const auto& obstacleRects = getObstacleRects();
    m_cachedSolidTopRects.insert(m_cachedSolidTopRects.end(), obstacleRects.begin(),
                                 obstacleRects.end());
    // Append platform rects
    const auto& platformRects = getPlatformRects();
    m_cachedSolidTopRects.insert(m_cachedSolidTopRects.end(), platformRects.begin(),
                                 platformRects.end());
    return m_cachedSolidTopRects;
}

void World::render(sf::RenderTarget& target) const {
    const sf::View oldView = target.getView();
    target.setView(m_view);

    m_environment.renderBackground(target, m_view);

    for (const std::unique_ptr<Entity>& pEntity : m_entities) {
        if (dynamic_cast<Actor*>(pEntity.get()))
            pEntity->render(target);
    }

    m_environment.renderForeground(target, m_view);

    for (const std::unique_ptr<Entity>& pEntity : m_entities) {
        if (!dynamic_cast<Actor*>(pEntity.get()))
            pEntity->render(target);
    }

    // Debug helpers
    if constexpr (Config::kDebugDraw) {
        Debug::drawCameraGuides(target, m_view, getCameraCatchupX(), getFollowThresholdX());
    }

    target.setView(oldView);
}

float World::getCameraLeft() const { return m_view.getCenter().x - 0.5f * m_view.getSize().x; }

float World::getCameraCatchupX() const { return getCameraLeft() + kCatchupMarginLeft; }

float World::getFollowThresholdX() const {
    return getCameraLeft() + kFollowThresholdRatio * m_view.getSize().x;
}

sf::Vector2f World::getMouseWorld() const {
    const sf::Vector2i pixel = sf::Mouse::getPosition(m_window);
    return m_window.mapPixelToCoords(pixel, m_view);
}

void World::addScore(int points) { m_session.addScore(points); }

void World::requestExitToMenu() { m_owner.requestExitToMenu(); }
