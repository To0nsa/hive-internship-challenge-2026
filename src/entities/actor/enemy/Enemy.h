#pragma once

#include "entities/actor/Actor.h"

class Enemy : public Actor {
  public:
    Enemy()           = default;
    ~Enemy() override = default;
};
