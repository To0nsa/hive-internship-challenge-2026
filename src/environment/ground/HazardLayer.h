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
    explicit HazardLayer(const HazardConfig& cfg) : m_cfg(cfg) {}

    void update(float dt) {
        if (m_animator)
            m_animator->update(dt);
    }

    bool hasHazard() const { return m_cfg.type != HazardType::None; }

    void drawForView(sf::RenderTarget& target, const sf::View& /*view*/,
                     const std::vector<sf::FloatRect>& gaps) const {
        if (m_cfg.type == HazardType::None)
            return;

        // Make sure animation is set up
        ensureClipExists();

        if (!m_sprite)
            return;

        // --- Prepare sprite transform (origin + scale) based on current frame ---
        const sf::IntRect frame  = m_sprite->getTextureRect();
        const float       frameW = static_cast<float>(std::max(1, frame.size.x));
        const float       frameH = static_cast<float>(std::max(1, frame.size.y));
        // Center origin so scaling/positioning uses the sprite center
        m_sprite->setOrigin({frameW * 0.5f, frameH * 0.5f});
        // Scale up to match size of gap visually
        const float scale = m_cfg.scale;
        m_sprite->setScale({scale, scale});

        // Loop over gaps and draw hazard in each
        for (const auto& gap : gaps) {
            const float centerX = gap.position.x + 0.5f * gap.size.x;
            const float centerY = gap.position.y + 0.5f * gap.size.y - m_cfg.yOffset;

            m_sprite->setPosition({centerX, centerY});
            target.draw(*m_sprite);
        }
    }

    bool intersectsHazard(const sf::FloatRect& aabb,
                          const std::vector<sf::FloatRect>& gaps) const {
        if (m_cfg.type == HazardType::None)
            return false;

        for (const auto& gap : gaps) {
            if (geom::aabbIntersects(aabb, gap))
                return true;
        }
        return false;
    }

  private:
    // Lazily create hazard animation clip & sprite the first time we need it.
    void ensureClipExists() const {
        if (m_animator || m_cfg.type == HazardType::None)
            return; // Already initialized or unused

        // Load hazard texture (fall back to lava texture if not provided for Lava)
        const std::string_view path = !m_cfg.texturePath.empty()
                                          ? m_cfg.texturePath
                                          : Assets::Tex::Environment::Ground::Lava;
        sf::Texture& tex = ResourceManager::getTexture(path);

        // Build an animation clip from a sheet (50x50 frames, first 2x2 area, 6 fps, looping)
        m_clip = Animation::makeClipFromSheet("hazard", tex, {50, 50}, {0, 0}, {1, 1}, 6.f, true);

        // Create sprite + animator bound to that sprite
        m_sprite   = std::make_unique<sf::Sprite>(tex);
        m_animator = std::make_unique<SpriteAnimator>(*m_sprite);

        // Register clip and start playing it
        m_clipId = m_animator->addClip(m_clip);
        m_animator->playClip(m_clipId);
    }

  private:
    HazardConfig m_cfg;

    mutable std::unique_ptr<sf::Sprite>     m_sprite;
    mutable std::unique_ptr<SpriteAnimator> m_animator;
    mutable AnimationClip                   m_clip;
    mutable SpriteAnimator::ClipId          m_clipId = SpriteAnimator::kInvalidClip;
};

