#pragma once

#include "animation/Animation.h"
#include "core/Assets.h"
#include "core/ResourceManager.h"
#include "environment/ground/GroundTypes.h"
#include "utils/Geom.h"
#include "utils/Math.h"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <limits>
#include <memory>
#include <vector>

// Handles rendering and collision detection for hazards that occupy ground gaps.
class HazardLayer {
  public:
    explicit HazardLayer(const HazardConfig& config) : m_cfg(config) {}

    // Call once after construction.
    bool init() {
        if (!m_cfg.enabled)
            return true; // Nothing to set up.

        if (m_cfg.lava.texturePath.empty())
            return true; // Only hole hazards enabled; no texture to load.

        sf::Texture& texture = ResourceManager::getTexture(m_cfg.lava.texturePath);

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

    void update(float dt) {
        if (m_animator)
            m_animator->update(dt);
    }

    bool hasHazard() const { return m_cfg.enabled; }

    void drawForView(sf::RenderTarget&             target, const sf::View& /*view*/,
                     const std::vector<GroundGap>& gaps) {
        if (!m_cfg.enabled)
            return;

        for (const auto& gap : gaps) {
            const HazardType type = pickHazardForBlock(gap.blockIndex);
            switch (type) {
            case HazardType::Hole:
                drawHoleGap(target, gap.rect);
                break;
            case HazardType::Lava:
                drawLavaGap(target, gap.rect);
                break;
            default:
                break;
            }
        }
    }

    bool intersectsHazard(const sf::FloatRect& aabb, const std::vector<GroundGap>& gaps) const {
        if (!m_cfg.enabled)
            return false;

        for (const auto& gap : gaps) {
            if (geom::aabbIntersects(aabb, gap.rect))
                return true;
        }
        return false;
    }

  private:
    HazardType pickHazardForBlock(int blockIndex) const {
        if (!m_cfg.enabled)
            return HazardType::None;

        const float clampedHoleChance = std::clamp(m_cfg.holeChance, 0.f, 1.f);
        if (clampedHoleChance <= 0.f)
            return HazardType::Lava;
        if (clampedHoleChance >= 1.f)
            return HazardType::Hole;

        const std::uint32_t hash       = math::mix32(static_cast<std::uint32_t>(blockIndex));
        const float         unitRandom = static_cast<float>(hash) /
                                 static_cast<float>(std::numeric_limits<std::uint32_t>::max());
        if (unitRandom < clampedHoleChance)
            return HazardType::Hole;

        return HazardType::Lava;
    }

    void drawLavaGap(sf::RenderTarget& target, const sf::FloatRect& gap) const {
        if (!m_sprite)
            return;

        const sf::IntRect frame       = m_sprite->getTextureRect();
        const float       frameWidth  = static_cast<float>(std::max(1, frame.size.x));
        const float       frameHeight = static_cast<float>(std::max(1, frame.size.y));

        m_sprite->setOrigin({frameWidth * 0.5f, frameHeight * 0.5f});
        m_sprite->setScale({m_cfg.lava.scale, m_cfg.lava.scale});

        const float centerX = gap.position.x + 0.5f * gap.size.x;
        const float centerY = gap.position.y + 0.5f * gap.size.y - m_cfg.lava.yOffset;

        m_sprite->setPosition({centerX, centerY});
        target.draw(*m_sprite);
    }

    void drawHoleGap(sf::RenderTarget& target, const sf::FloatRect& gap) const {
        sf::RectangleShape holeShape;
        holeShape.setPosition(gap.position);
        holeShape.setSize(gap.size);
        holeShape.setFillColor(sf::Color::Black);
        target.draw(holeShape);
    }

    HazardConfig m_cfg;

    std::unique_ptr<sf::Sprite>     m_sprite;
    std::unique_ptr<SpriteAnimator> m_animator;
    AnimationClip                   m_clip;
    SpriteAnimator::ClipId          m_clipId = SpriteAnimator::kInvalidClip;
};
