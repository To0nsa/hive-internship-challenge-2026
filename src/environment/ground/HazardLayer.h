#pragma once

#include "animation/Animation.h"
#include "core/Assets.h"
#include "core/ResourceManager.h"
#include "environment/ground/GroundTypes.h"
#include "utils/Geom.h"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <memory>
#include <vector>

// Handles rendering and collision detection for hazards that occupy ground gaps.
class HazardLayer {
  public:
    explicit HazardLayer(const HazardConfig& config) : m_cfg(config) {}

    // Call once after construction.
    bool init() {
        if (m_cfg.type == HazardType::None)
            return true; // Nothing to set up, but not an error.

        sf::Texture& texture = ResourceManager::getTexture(m_cfg.texturePath);

        // Build an animation clip from a sheet (50x50 frames, first 2x2 area, 6 fps, looping).
        m_clip = Animation::makeClipFromSheet("hazard", texture, {50, 50}, // frame size
                                              {0, 0},                      // first cell
                                              {1, 1},                      // last cell
                                              6.f,                         // fps
                                              true);                       // looping

        // Create sprite + animator bound to that sprite.
        m_sprite   = std::make_unique<sf::Sprite>(texture);
        m_animator = std::make_unique<SpriteAnimator>(*m_sprite);

        // Register clip and start playing it.
        m_clipId = m_animator->addClip(m_clip);
        m_animator->playClip(m_clipId);

        return true;
    }

    void update(float dt) { m_animator->update(dt); }

    bool hasHazard() const { return m_cfg.type != HazardType::None; }

    void drawForView(sf::RenderTarget&                 target, const sf::View& /*view*/,
                     const std::vector<sf::FloatRect>& gaps) {
        if (m_cfg.type == HazardType::None)
            return;

        // Prepare sprite transform (origin + scale) based on current frame.
        const sf::IntRect frame       = m_sprite->getTextureRect();
        const float       frameWidth  = static_cast<float>(std::max(1, frame.size.x));
        const float       frameHeight = static_cast<float>(std::max(1, frame.size.y));

        // Center origin so scaling/positioning uses the sprite center.
        m_sprite->setOrigin({frameWidth * 0.5f, frameHeight * 0.5f});

        // Scale up to match size of gap visually.
        const float scale = m_cfg.scale;
        m_sprite->setScale({scale, scale});

        // Loop over gaps and draw hazard in each.
        for (const auto& gap : gaps) {
            const float centerX = gap.position.x + 0.5f * gap.size.x;
            const float centerY = gap.position.y + 0.5f * gap.size.y - m_cfg.yOffset;

            m_sprite->setPosition({centerX, centerY});
            target.draw(*m_sprite);
        }
    }

    bool intersectsHazard(const sf::FloatRect& aabb, const std::vector<sf::FloatRect>& gaps) const {
        if (m_cfg.type == HazardType::None)
            return false;

        for (const auto& gap : gaps) {
            if (geom::aabbIntersects(aabb, gap))
                return true;
        }
        return false;
    }

  private:
    HazardConfig m_cfg;

    std::unique_ptr<sf::Sprite>     m_sprite;
    std::unique_ptr<SpriteAnimator> m_animator;
    AnimationClip                   m_clip;
    SpriteAnimator::ClipId          m_clipId = SpriteAnimator::kInvalidClip;
};
