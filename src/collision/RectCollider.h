#pragma once

#include "Collider.h"

class Entity;

class RectCollider final : public Collider {
  public:
    explicit RectCollider(const Entity& entity) : m_entity(&entity) {}

    sf::FloatRect worldAabb() const override;

  private:
    const Entity* m_entity = nullptr;
};