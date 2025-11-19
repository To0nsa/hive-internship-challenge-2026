#pragma once

#include "../Config.h"
#include "../Debug.h"
#include "../ResourceManager.h"
#include "../animation/Animation.h"
#include "../collision/MultiRectCollider.h"
#include "../utils/Geom.h"
#include "../utils/Math.h"
#include "BackgroundAssets.h"
#include "StripUtil.h"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class GroundStream : public sf::Drawable {
  public:
    explicit GroundStream(const strip::ParallaxLayerDesc& layer) : m_layer(layer) {}

    void update(float dt) {
        if (m_lavaAnimator)
            m_lavaAnimator->update(dt);
    }

    // Build collidable rectangles for the current view.
    void updateForView(const sf::View& view) {
        // View geometry
        const sf::Vector2f viewSize = view.getSize();
        const sf::Vector2f center   = view.getCenter();

        const float viewLeft  = center.x - 0.5f * viewSize.x;
        const float viewRight = viewLeft + viewSize.x;

        const float height = viewSize.y * kColliderHeightRatio;
        const float y =
            (center.y - 0.5f * viewSize.y) + (viewSize.y - height); // bottom-anchored band

        // Visible block range
        const int firstCell  = static_cast<int>(std::floor(viewLeft / kCellWidth));
        const int lastCell   = static_cast<int>(std::ceil(viewRight / kCellWidth));
        const int firstBlock = blockIndexForCell(firstCell) - 1;
        const int lastBlock  = blockIndexForCell(lastCell) + 1;

        // Build solid rects for each block in view
        std::vector<sf::FloatRect> solids;
        solids.reserve(std::max(0, (lastBlock - firstBlock + 1)) * 2);

        for (int block = firstBlock; block <= lastBlock; ++block)
            appendSolidsForBlock(block, y, height, viewLeft, viewRight, solids);

        m_collider.setRectColliders(std::move(solids));
    }

    // Draws the textured ground strip + lava inside gaps + debug AABBs.
    void drawForView(sf::RenderTarget& target, const sf::View& view) const {
        const std::string file = bgassets::keyToFilename(m_layer.key);
        sf::Texture& pTex = ResourceManager::getTexture(file);
        // Ensure horizontal repeating for scrolling
        pTex.setRepeated(true);
        strip::drawStrip(target, view, pTex, m_layer.factor);
        drawLavaGaps(target, view);

        if constexpr (Config::kDebugDraw) {
            for (const auto& r : m_collider.getRectColliders()) {
                sf::RectangleShape outline;
                outline.setPosition(r.position);
                outline.setSize(r.size);
                outline.setFillColor(sf::Color(0, 0, 0, 0));
                outline.setOutlineColor(sf::Color::Yellow);
                outline.setOutlineThickness(1.f);
                target.draw(outline);
            }
        }
    }

    const MultiRectCollider& getCollider() const { return m_collider; }

    // Returns true if aabb overlaps the lava band inside any visible gap for the given view.
    bool intersectsLavaGap(const sf::FloatRect& aabb, const sf::View& view) const {
        // View geometry
        const sf::Vector2f viewSize = view.getSize();
        const sf::Vector2f center   = view.getCenter();

        const float viewL = center.x - 0.5f * viewSize.x;
        const float viewR = viewL + viewSize.x;

        const float bandH = viewSize.y * kColliderHeightRatio;
        const float y     = (center.y - 0.5f * viewSize.y) + (viewSize.y - bandH);

        // Visible block range
        const int firstCell  = static_cast<int>(std::floor(viewL / kCellWidth));
        const int lastCell   = static_cast<int>(std::ceil(viewR / kCellWidth));
        const int firstBlock = blockIndexForCell(firstCell) - 1;
        const int lastBlock  = blockIndexForCell(lastCell) + 1;

        sf::FloatRect gap;
        for (int block = firstBlock; block <= lastBlock; ++block) {
            gap = gapRectForBlock(block, y, bandH);
            if (geom::left(gap) + geom::width(gap) < viewL || geom::left(gap) > viewR)
                continue;
            if (geom::aabbIntersects(aabb, gap))
                return true;
        }
        return false;
    }

  private:
    void draw(sf::RenderTarget&, sf::RenderStates) const override {}

    // Lava animation setup
    void ensureLavaClipExists() const {
        if (m_lavaAnimator)
            return;
        sf::Texture& pTex = ResourceManager::getTexture("lava.png");
        if (&pTex) {
            m_lavaClip =
                Animation::makeClipFromSheet("lava", pTex, {50, 50}, {0, 0}, {1, 1}, 6.f, true);
            m_pLavaSprite = std::make_unique<sf::Sprite>(pTex);
        } else {
            // Texture missing; create an empty sprite to keep animator construction safe
            static sf::Texture dummy;
            m_pLavaSprite = std::make_unique<sf::Sprite>(dummy);
        }
        m_lavaAnimator = std::make_unique<SpriteAnimator>(*m_pLavaSprite);
        m_lavaAnimator->addClip(m_lavaClip);
        m_lavaAnimator->playClip("lava");
    }

    // Rendering of lava inside gaps
    void drawLavaGaps(sf::RenderTarget& target, const sf::View& view) const {
        ensureLavaClipExists();

        // View geometry
        const sf::Vector2f viewSize = view.getSize();
        const sf::Vector2f center   = view.getCenter();

        const float viewL = center.x - 0.5f * viewSize.x;
        const float viewR = viewL + viewSize.x;

        const float bandH = viewSize.y * kColliderHeightRatio;
        // Slight lift so the lava reads above the band visually
        const float y = (center.y - 0.5f * viewSize.y) + (viewSize.y - bandH) - kLavaYOffset;

        // Same visible block range as updateForView
        const int firstCell  = static_cast<int>(std::floor(viewL / kCellWidth));
        const int lastCell   = static_cast<int>(std::ceil(viewR / kCellWidth));
        const int firstBlock = blockIndexForCell(firstCell) - 1;
        const int lastBlock  = blockIndexForCell(lastCell) + 1;

        // Prepare lava sprite
        if (!m_pLavaSprite)
            return;
        const sf::IntRect frame  = m_pLavaSprite->getTextureRect();
        const float       frameW = static_cast<float>(std::max(1, frame.size.x));
        const float       frameH = static_cast<float>(std::max(1, frame.size.y));
        m_pLavaSprite->setOrigin({frameW * 0.5f, frameH * 0.5f});
        m_pLavaSprite->setScale({kLavaScale, kLavaScale});

        for (int block = firstBlock; block <= lastBlock; ++block) {
            const sf::FloatRect gap = gapRectForBlock(block, y, bandH);
            if (geom::left(gap) + geom::width(gap) < viewL || geom::left(gap) > viewR)
                continue;

            const float centerX = geom::left(gap) + 0.5f * geom::width(gap);
            const float centerY = y + 0.5f * bandH;

            m_pLavaSprite->setPosition({centerX, centerY});
            target.draw(*m_pLavaSprite);
        }
    }

    // Block / gap helpers
    // Converts a cell index to its containing block index.
    static int blockIndexForCell(int cell) { return math::floorDivInt(cell, kBlockCells); }
    // Returns the world-space anchor cell index for the block.
    static int blockAnchor(int blockIdx) { return blockIdx * kBlockCells; }

    // Returns the world-space gap rect for the block (width = one cell).
    static sf::FloatRect gapRectForBlock(int blockIdx, float y, float h) {
        const int           anchor    = blockAnchor(blockIdx);
        const std::uint32_t random    = math::mix32(static_cast<std::uint32_t>(anchor));
        const int           gapOffset = 1 + static_cast<int>(random % 4); // 1..4
        const float         left      = (anchor + gapOffset) * kCellWidth;
        return sf::FloatRect{{left, y}, {kCellWidth, h}};
    }

    // Appends the two solid segments of a block (left & right of the gap) clipped to [viewL,
    // viewR].
    static void appendSolidsForBlock(int blockIdx, float y, float h, float viewL, float viewR,
                                     std::vector<sf::FloatRect>& out) {
        // Block bounds
        const int   anchor = blockAnchor(blockIdx);
        const float blockL = anchor * kCellWidth;
        const float blockR = (anchor + kBlockCells) * kCellWidth;

        // Clamp to view
        const sf::FloatRect gap  = gapRectForBlock(blockIdx, y, h);
        const float         gapL = geom::left(gap);
        const float         gapR = geom::left(gap) + geom::width(gap);

        // Helper to push a clipped rect
        auto clipPush = [&](float L, float R) {
            const float left  = std::max(L, viewL);
            const float right = std::min(R, viewR);
            if (right > left)
                out.emplace_back(sf::FloatRect{{left, y}, {right - left, h}});
        };

        // Push solid segments
        clipPush(blockL, gapL); // left solid
        clipPush(gapR, blockR); // right solid
    }

  public:
    // Top Y of the collidable band for a given view
    float getTopYForView(const sf::View& view) const {
        const sf::Vector2f viewSize = view.getSize();
        const sf::Vector2f center   = view.getCenter();
        const float        bandH    = viewSize.y * kColliderHeightRatio;
        const float        y        = (center.y - 0.5f * viewSize.y) + (viewSize.y - bandH);
        return y;
    }

  private:
    strip::ParallaxLayerDesc m_layer;
    MultiRectCollider        m_collider;

    // Lava animation state.
    mutable std::unique_ptr<sf::Sprite>     m_pLavaSprite;
    mutable std::unique_ptr<SpriteAnimator> m_lavaAnimator;
    mutable AnimationClip                   m_lavaClip;

    // Tunables (kept close to original intent)
    static inline constexpr float kColliderHeightRatio = 0.05f; // fraction of view height
    static inline constexpr float kCellWidth           = 220.f; // world units per cell
    static inline constexpr int   kBlockCells          = 5;     // cells per block (1 gap inside)
    static inline constexpr float kLavaScale           = 6.f;   // sprite scale
    static inline constexpr float kLavaYOffset         = 85.f;  // visual lift above band
};
