#include "collision/RectCollider.h"

#include "entities/Entity.h"

#include <SFML/Graphics/Transformable.hpp>
#include <cmath>

sf::FloatRect RectCollider::worldAabb() const {
    const sf::Transformable& sprite = m_entity->getColliderTransformable();
    const sf::Vector2f&      size   = m_entity->getColliderSize();
    const sf::Vector2f&      offset = m_entity->getColliderOffset();

    const sf::Vector2f center = sprite.getTransform().transformPoint(sprite.getOrigin() + offset);

    const sf::Vector2f s = sprite.getScale();
    const sf::Vector2f half{std::abs(s.x) * (size.x * 0.5f), std::abs(s.y) * (size.y * 0.5f)};

    const sf::Vector2f pos  = {center.x - half.x, center.y - half.y};
    const sf::Vector2f dims = {half.x * 2.f, half.y * 2.f};

    return sf::FloatRect(pos, dims);
}
