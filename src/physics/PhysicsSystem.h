#pragma once

#include "physics/PhysicsBody.h"

#include <SFML/System/Vector2.hpp>
#include <vector>

struct StaticWorldGeometry;

class Entity;

// Lightweight physics system responsible for integrating velocities and
// resolving collisions against static world solids (ground, obstacles, platforms).
//
// It operates purely on Entity position/velocity and collider AABBs; gameplay
// systems remain responsible for damage, scoring, etc.
class PhysicsSystem {
  public:
    PhysicsSystem()  = default;
    ~PhysicsSystem() = default;

    // Registers a body for the given entity. If the entity is already registered,
    // its config is updated.
    void registerBody(Entity& owner, const PhysicsBodyConfig& config);

    // Removes any body associated with the given entity.
    void unregisterBody(const Entity& owner);

    // Returns the physics body for the entity, or nullptr if none.
    PhysicsBody*       findBody(Entity& owner);
    const PhysicsBody* findBody(const Entity& owner) const;

    // Steps physics for all registered bodies.
    //
    // dt:          frame delta time in seconds.
    // staticWorld: combined geometry for all static solids (ground, obstacles, platforms).
    void step(float dt, const StaticWorldGeometry* staticWorld);

    // Global gravity magnitude in pixels per second^2.
    // Bodies use this multiplied by their gravityScale when useGravity is true.
    void  setGravity(float g) { m_gravity = g; }
    float getGravity() const { return m_gravity; }

  private:
    std::vector<PhysicsBody> m_bodies;

    float m_gravity = 2400.f;

    // Integrates a single body for one frame: forces, position, and contacts.
    void integrateBody(PhysicsBody& body, float dt, const StaticWorldGeometry* staticWorld);

    // Apply per-body forces (currently gravity) and clamp velocity.
    void applyForces(PhysicsBody& body, sf::Vector2f& velocity, float dt);

    // Integrate position from velocity and write back via Entity::setPosition.
    void integratePosition(Entity& owner, sf::Vector2f& position, const sf::Vector2f& velocity,
                           float dt);

    // Resolve vertical top-only contacts against static world and update grounded/velocity.
    void resolveTopOnlyGround(PhysicsBody& body, Entity& owner, sf::Vector2f& position,
                              sf::Vector2f& velocity, float dy,
                              const StaticWorldGeometry& staticWorld);

    // Resolve horizontal collisions against static world walls (left/right sides of solids).
    void resolveHorizontalCollisions(PhysicsBody& body, Entity& owner, sf::Vector2f& position,
                                     sf::Vector2f&              velocity,
                                     const StaticWorldGeometry& staticWorld);
};
