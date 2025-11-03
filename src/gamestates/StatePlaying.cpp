#include "StatePlaying.h"

#include "../Config.h"
#include "../Debug.h"
#include "../ResourceManager.h"
#include "../collision/MultiRectCollider.h"
#include "../entities/actor/Actor.h"
#include "../obstacle/Obstacle.h"
#include "../obstacle/ObstacleFactory.h"
#include "../obstacle/ObstacleTypes.h"
#include "../utils/Math.h"
#include "../utils/Random.h"
#include "StatePaused.h"
#include "StateStack.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>

StatePlaying::StatePlaying(StateStack& stateStack) : m_stateStack(stateStack) {}

bool StatePlaying::init() {
    // View set to window size
    m_view.setSize(
        {static_cast<float>(Config::windowWidth), static_cast<float>(Config::windowHeight)});
    m_view.setCenter({m_view.getSize().x * 0.5f, m_view.getSize().y * 0.5f});
    // Camera state
    m_cameraX       = m_view.getCenter().x;
    m_cameraTargetX = m_cameraX;
    m_cameraSpeed   = 0.f; // ease-in

    // Backgrounds
    m_bg = std::make_unique<ParallaxBackground>(std::initializer_list<strip::ParallaxLayerDesc>{
        {"bg_01", 0.16f},
        {"bg_02", 0.22f},
        {"bg_03", 0.28f},
        {"bg_04", 0.34f},
        {"bg_05", 0.50f},
        {"bg_06", 0.62f},
        {"bg_08", 0.76f},
    });
    // Animated background strip
    m_bgAnim = std::make_unique<AnimatedParallaxStrip>(
        std::vector<std::string>{"bg_anim_01", "bg_anim_02", "bg_anim_03"}, 0.15f, 6.f);

    // Ground stream with colliders
    m_ground = std::make_unique<GroundStream>(strip::ParallaxLayerDesc{"bg_07", 1.f});
    m_ground->updateForView(m_view);

    // Create player entity
    m_pPlayer = createEntity<Player>();
    if (!m_pPlayer || !m_pPlayer->init())
        return false;
    m_pPlayer->setPosition(sf::Vector2f(200, 800));

    // Seed RNG and schedule first obstacle spawn a bit ahead of view
    Random::seed(Random::timeSeed());
    const float viewRight = getCameraLeft() + m_view.getSize().x * 2.f;
    m_nextObstacleX       = viewRight + Random::rangef(680.f, 1280.f);

    return true;
}

void StatePlaying::update(float dt) {
    bool isPauseKeyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);
    m_hasPauseKeyBeenReleased |= !isPauseKeyPressed;
    if (m_hasPauseKeyBeenReleased && isPauseKeyPressed) {
        m_hasPauseKeyBeenReleased = false;
        m_stateStack.push<StatePaused>();
    }

    // Update background anim
    if (m_bgAnim)
        m_bgAnim->update(dt);

    // Update all entities
    for (const std::unique_ptr<Entity>& pEntity : m_entities)
        pEntity->update(dt);

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

    // Update ground colliders for current view
    if (m_ground)
        m_ground->updateForView(m_view);

    // Apply physics to actors using combined ground + obstacle colliders (walk on them)
    MultiRectCollider combined;
    if (m_ground) {
        std::vector<sf::FloatRect> solids = m_ground->getCollider().getRectColliders();
        solids.reserve(solids.size() + m_entities.size());
        for (auto& entity : m_entities) {
            if (!entity->isAlive())
                continue;
            if (auto* o = dynamic_cast<Obstacle*>(entity.get())) {
                solids.emplace_back(o->getCollider().worldAabb());
            }
        }
        combined.setRectColliders(std::move(solids));
    }
    for (auto& entity : m_entities) {
        if (!entity->isAlive())
            continue;
        if (auto* actor = dynamic_cast<Actor*>(entity.get())) {
            const Collider* col = m_ground ? static_cast<const Collider*>(&combined)
                                           : static_cast<const Collider*>(nullptr);
            actor->applyPhysics(dt, col);
        }
    }

    // Horizontal collide player vs obstacles
    if (m_pPlayer && m_pPlayer->isAlive()) {
        sf::FloatRect pb = m_pPlayer->getCollider().worldAabb();
        for (auto& entity : m_entities) {
            if (!entity->isAlive())
                continue;
            if (auto* o = dynamic_cast<Obstacle*>(entity.get())) {
                const sf::FloatRect ob = o->getCollider().worldAabb();
                sf::FloatRect       inter;
                if (geom::aabbIntersects(pb, ob, inter)) {
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

    // Stream simple random obstacles ahead of camera
    {
        const float viewLeft  = getCameraLeft();
        const float viewRight = viewLeft + m_view.getSize().x * 2.f;
        if (m_ground) {
            const float groundTop = m_ground->getTopYForView(m_view);
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

    if (m_ground)
        m_ground->updateForView(m_view);
}

// Collect obstacle rectangles for collision queries
const std::vector<sf::FloatRect>& StatePlaying::getObstacleRects() const {
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

// Collect all walkable-top solids: ground + obstacles
const std::vector<sf::FloatRect>& StatePlaying::getSolidTopRects() const {
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
    return m_cachedSolidTopRects;
}

void StatePlaying::render(sf::RenderTarget& target) const {
    const sf::View oldView = target.getView();
    target.setView(m_view);

    if (m_bgAnim)
        m_bgAnim->drawForView(target, m_view);

    if (m_bg) {
        const std::size_t i06  = m_bg->findIndexByKey("bg_06");
        const std::size_t upto = (i06 < m_bg->size()) ? i06 : (m_bg->size() ? m_bg->size() - 1 : 0);
        m_bg->drawRangeForView(target, m_view, 0, upto);
    }

    for (const std::unique_ptr<Entity>& pEntity : m_entities) {
        if (dynamic_cast<Actor*>(pEntity.get()))
            pEntity->render(target);
    }

    if (m_ground)
        m_ground->drawForView(target, m_view);

    for (const std::unique_ptr<Entity>& pEntity : m_entities) {
        if (!dynamic_cast<Actor*>(pEntity.get()))
            pEntity->render(target);
    }

    // Foreground layer 08 after entities
    if (m_bg) {
        const std::size_t i08 = m_bg->findIndexByKey("bg_08");
        if (i08 < m_bg->size())
            m_bg->drawRangeForView(target, m_view, i08, i08);
    }

    // Debug helpers
    if constexpr (Config::kDebugDraw) {
        Debug::drawCameraGuides(target, m_view, getCameraCatchupX(), getFollowThresholdX());
    }

    target.setView(oldView);
}

float StatePlaying::getCameraLeft() const {
    return m_view.getCenter().x - 0.5f * m_view.getSize().x;
}

float StatePlaying::getCameraCatchupX() const { return getCameraLeft() + kCatchupMarginLeft; }

float StatePlaying::getFollowThresholdX() const {
    return getCameraLeft() + kFollowThresholdRatio * m_view.getSize().x;
}
