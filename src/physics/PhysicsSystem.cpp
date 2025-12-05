#include "physics/PhysicsSystem.h"

#include "collision/MultiRectCollider.h"
#include "collision/RectCollider.h"
#include "entities/Entity.h"
#include "utils/Geom.h"
#include "utils/Math.h"

#include <SFML/Graphics/Rect.hpp>
#include <algorithm>
#include <limits>

void PhysicsSystem::registerBody(Entity& owner, const PhysicsBodyConfig& config) {
    // If already present, just update config.
    for (auto& body : m_bodies) {
        if (body.owner == &owner) {
            body.config = config;
            return;
        }
    }

    PhysicsBody body;
    body.owner    = &owner;
    body.config   = config;
    body.grounded = false;
    m_bodies.push_back(body);
}

void PhysicsSystem::unregisterBody(const Entity& owner) {
    m_bodies.erase(
        std::remove_if(m_bodies.begin(), m_bodies.end(),
                       [&owner](const PhysicsBody& body) { return body.owner == &owner; }),
        m_bodies.end());
}

PhysicsBody* PhysicsSystem::findBody(Entity& owner) {
    for (auto& body : m_bodies) {
        if (body.owner == &owner)
            return &body;
    }
    return nullptr;
}

const PhysicsBody* PhysicsSystem::findBody(const Entity& owner) const {
    for (const auto& body : m_bodies) {
        if (body.owner == &owner)
            return &body;
    }
    return nullptr;
}

void PhysicsSystem::step(float dt, const MultiRectCollider* staticWorld) {
    if (dt <= 0.f || !staticWorld)
        return;

    for (auto& body : m_bodies) {
        if (!body.owner || !body.config.enabled)
            continue;
        if (!body.owner->isAlive())
            continue;

        integrateBody(body, dt, staticWorld);
    }
}

void PhysicsSystem::integrateBody(PhysicsBody& body, float dt,
                                  const MultiRectCollider* staticWorld) {
    Entity& owner = *body.owner;

    if (body.config.isKinematic) {
        // Kinematic bodies are controlled entirely by gameplay code.
        // Clear grounded flag; callers can override if they treat themselves as grounded.
        body.grounded = false;
        return;
    }

    // Fetch current state from the entity.
    sf::Vector2f position = owner.getPosition();
    sf::Vector2f velocity = owner.getVelocity();

    // Forces + clamping.
    applyForces(body, velocity, dt);

    // Cache vertical delta for ground resolution.
    const float dy = velocity.y * dt;

    body.grounded = false;

    // Integrate position on both axes.
    integratePosition(owner, position, velocity, dt);

    // Top-only ground resolution matches previous Actor::applyPhysics() behavior.
    if (body.config.topOnlyGround && velocity.y >= 0.f) {
        resolveTopOnlyGround(body, owner, position, velocity, dy, *staticWorld);
    }

    // Push updated velocity back to the entity.
    owner.setVelocity(velocity);
}

void PhysicsSystem::applyForces(PhysicsBody& body, sf::Vector2f& velocity, float dt) {
    // Apply gravity using global gravity magnitude scaled per body.
    if (body.config.useGravity) {
        velocity.y += m_gravity * body.config.gravityScale * dt;
    }

    // Clamp speeds to configured maximum magnitudes.
    velocity.x = math::clampToMagnitude(velocity.x, body.config.maxVelX);
    velocity.y = math::clampToMagnitude(velocity.y, body.config.maxVelY);
}

void PhysicsSystem::integratePosition(Entity& owner, sf::Vector2f& position,
                                      const sf::Vector2f& velocity, float dt) {
    const float dx = velocity.x * dt;
    const float dy = velocity.y * dt;

    if (dx != 0.f || dy != 0.f) {
        if (dx != 0.f) {
            position.x += dx;
            owner.setPosition(position);
        }

        if (dy != 0.f) {
            position.y += dy;
            owner.setPosition(position);
        }
    } else {
        // No movement this frame; still need collider below for precise grounding.
        owner.setPosition(position);
    }
}

void PhysicsSystem::resolveTopOnlyGround(PhysicsBody& body, Entity& owner, sf::Vector2f& position,
                                         sf::Vector2f& velocity, float dy,
                                         const MultiRectCollider& staticWorld) {
    const sf::FloatRect actorCollider = owner.getCollider().worldAabb();

    // Previous frame actor position on Y.
    sf::FloatRect prevActor = actorCollider;
    prevActor.position.y -= dy;

    float bestLiftDy  = -std::numeric_limits<float>::infinity();
    bool  touchingTop = false;

    auto consider = [&](const sf::FloatRect& groundCollider) {
        // Track exact top contact for zero-motion cases.
        if (velocity.y == 0.f && geom::touchTop(actorCollider, groundCollider, 0.75f))
            touchingTop = true;

        const float groundTop    = geom::top(groundCollider);
        const float prevBottom   = geom::bottom(prevActor);
        const float currentB     = geom::bottom(actorCollider);
        const float currentLeft  = geom::left(actorCollider);
        const float currentRight = geom::right(actorCollider);
        const float groundLeft   = geom::left(groundCollider);
        const float groundRight  = geom::right(groundCollider);

        // Require horizontal overlap to stand on top.
        if (currentRight <= groundLeft || currentLeft >= groundRight)
            return;

        constexpr float kTopEps = 0.001f;
        if (prevBottom > groundTop + kTopEps)
            return; // not a landing from above
        if (currentB < groundTop - kTopEps)
            return; // still above top

        const float deltaY = groundTop - currentB;
        if (deltaY > bestLiftDy)
            bestLiftDy = deltaY;
    };

    for (const auto& r : staticWorld.getRectColliders()) {
        consider(r);
    }

    if (bestLiftDy > -std::numeric_limits<float>::infinity()) {
        position.y += bestLiftDy;
        owner.setPosition(position);
        velocity.y    = 0.f;
        body.grounded = true;
    } else if (touchingTop) {
        body.grounded = true;
    }
}
