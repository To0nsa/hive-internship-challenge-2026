#pragma once

#include "collision/MultiRectCollider.h"

#include <SFML/Graphics/View.hpp>
#include <vector>

class Actor;
class Enemy;
class Environment;
class GameSession;
class Obstacle;
class Platform;
class Player;
class Projectile;
class RedSquare;
class World;

struct CollisionContext {
    float dt = 0.f;

    Player* player = nullptr;

    std::vector<Actor*>      actors;
    std::vector<Enemy*>      enemies;
    std::vector<Projectile*> projectiles;
    std::vector<Obstacle*>   obstacles;
    std::vector<Platform*>   platforms;
    std::vector<RedSquare*>  collectibles;

    const MultiRectCollider* ground     = nullptr;
    const sf::View*          cameraView = nullptr;
    float                    cameraLeft = 0.f;

    GameSession*       session     = nullptr;
    const Environment* environment = nullptr;
};

namespace collision {

    // Resolves all gameplay collisions in a frame using the provided context.
    void resolve(const CollisionContext& ctx);

    // Builds a CollisionContext for the given world snapshot.
    CollisionContext buildCollisionContext(const World& world, float dt,
                                           const MultiRectCollider* groundCollider);

} // namespace collision
