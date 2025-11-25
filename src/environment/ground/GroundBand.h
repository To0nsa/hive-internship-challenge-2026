#pragma once

#include "collision/MultiRectCollider.h"
#include "environment/ground/GroundTypes.h"
#include "utils/Geom.h"
#include "utils/Math.h"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/View.hpp>
#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>

// Collision-only representation of the ground band at the bottom of the view.
// Responsible for building the walkable collider and exposing gap rectangles.
class GroundBand {
  public:
    GroundBand(float bandHeightRatio, float cellWidth, int cellsPerBlock, bool hasGaps,
               GapPatternFn gapPattern = {})
        : m_bandHeightRatio(bandHeightRatio), m_cellWidth(cellWidth),
          m_cellsPerBlock(cellsPerBlock), m_hasGaps(hasGaps), m_gapPattern(gapPattern) {}

    // Rebuild the set of collidable ground rectangles for the current camera view.
    void updateForView(const sf::View& view) {
        // --- View geometry (world space) ---
        const sf::Vector2f viewSize = view.getSize();
        const sf::Vector2f center   = view.getCenter();

        // Left/right edges in world coords
        const float viewLeft  = center.x - 0.5f * viewSize.x;
        const float viewRight = viewLeft + viewSize.x;

        // Ground collision band height = fixed fraction of view height
        const float height = viewSize.y * m_bandHeightRatio;

        // Y of the top of the band, stuck against bottom of view
        const float y =
            (center.y - 0.5f * viewSize.y) + (viewSize.y - height); // bottom-anchored band

        // --- Convert visible range to blocks ---

        // First / last cell indices covering the view horizontally
        const int firstCell = static_cast<int>(std::floor(viewLeft / m_cellWidth));
        const int lastCell  = static_cast<int>(std::ceil(viewRight / m_cellWidth));

        // Extend one block on each side to avoid visual/collision popping at edges
        const int firstBlock = blockIndexForCell(firstCell) - 1;
        const int lastBlock  = blockIndexForCell(lastCell) + 1;

        // Build solid rects for all blocks intersecting the view
        std::vector<sf::FloatRect> solids;
        // Each block can contribute up to 2 rects if we have gaps (left + right of the gap)
        // or 1 rect per block if we have continuous ground.
        const int rectsPerBlock = m_hasGaps ? 2 : 1;
        solids.reserve(std::max(0, (lastBlock - firstBlock + 1)) * rectsPerBlock);

        for (int block = firstBlock; block <= lastBlock; ++block)
            appendSolidsForBlock(block, y, height, viewLeft, viewRight, solids);

        // Upload result into MultiRectCollider (takes ownership of the vector)
        m_collider.setRectColliders(std::move(solids));
    }

    // Access to ground collider used by physics / actors
    const MultiRectCollider& getCollider() const { return m_collider; }

    // Top Y of the collidable band for a given view.
    // This is used by other systems (e.g., spawning obstacles on top of ground).
    float getTopYForView(const sf::View& view) const {
        const sf::Vector2f viewSize = view.getSize();
        const sf::Vector2f center   = view.getCenter();
        const float        bandH    = viewSize.y * m_bandHeightRatio;

        // Same computation as in updateForView() to keep everything consistent.
        const float y = (center.y - 0.5f * viewSize.y) + (viewSize.y - bandH);
        return y;
    }

    // Computes gap rectangles for the given view in world space.
    void gapsForView(const sf::View& view, std::vector<sf::FloatRect>& outGaps) const {
        outGaps.clear();

        if (!m_hasGaps)
            return;

        // --- View geometry (world space) ---
        const sf::Vector2f viewSize = view.getSize();
        const sf::Vector2f center   = view.getCenter();

        const float viewL = center.x - 0.5f * viewSize.x;
        const float viewR = viewL + viewSize.x;

        const float bandH = viewSize.y * m_bandHeightRatio;
        const float y     = (center.y - 0.5f * viewSize.y) + (viewSize.y - bandH);

        const int firstCell  = static_cast<int>(std::floor(viewL / m_cellWidth));
        const int lastCell   = static_cast<int>(std::ceil(viewR / m_cellWidth));
        const int firstBlock = blockIndexForCell(firstCell) - 1;
        const int lastBlock  = blockIndexForCell(lastCell) + 1;

        for (int block = firstBlock; block <= lastBlock; ++block) {
            const std::optional<sf::FloatRect> gapOpt = computeGapForBlock(block, y, bandH);
            if (!gapOpt)
                continue;

            const sf::FloatRect& gap = *gapOpt;

            // Skip if gap is completely off-screen
            if (geom::left(gap) + geom::width(gap) < viewL || geom::left(gap) > viewR)
                continue;

            outGaps.push_back(gap);
        }
    }

  private:
    // Converts a cell index to its containing block index.
    // Each block is m_cellsPerBlock cells wide.
    int blockIndexForCell(int cell) const { return math::floorDivInt(cell, m_cellsPerBlock); }

    // Returns the "anchor" cell index for this block (leftmost cell).
    int blockAnchor(int blockIdx) const { return blockIdx * m_cellsPerBlock; }

    // Returns the world-space rectangle of the gap within this block, if any.
    // - Exactly one gap per block when m_hasGaps is true and gapPattern does not override
    // - Gap width = one cell (m_cellWidth) by default
    // - Gap position inside the block is deterministic pseudo-random based on block index
    std::optional<sf::FloatRect> computeGapForBlock(int blockIdx, float y, float h) const {
        if (!m_hasGaps)
            return std::nullopt;

        // Degenerate case: a "block" of a single cell can't host an in-block gap,
        // treat it as solid.
        if (m_cellsPerBlock <= 1)
            return std::nullopt;

        if (m_gapPattern) {
            return m_gapPattern(blockIdx, y, h, m_cellWidth, m_cellsPerBlock);
        }

        const int anchor = blockAnchor(blockIdx);

        // Hash the anchor so gaps are pseudo-random but deterministic
        const std::uint32_t random = math::mix32(static_cast<std::uint32_t>(anchor));

        // Choose an offset in [1..cellsPerBlock-1] -> keeps gap away from anchor cell
        const int positions = m_cellsPerBlock - 1;
        const int gapOffset = 1 + static_cast<int>(random % positions);

        // X coordinate of gap's left edge (cell-based)
        const float left = (anchor + gapOffset) * m_cellWidth;

        // Rect representing the whole vertical band in that gap cell
        return sf::FloatRect{{left, y}, {m_cellWidth, h}};
    }

    // Appends solid segments of a block, clipped to [viewL, viewR].
    // If there is a gap, produces 2 rects (left & right of gap).
    // Otherwise, produces 1 rect covering the entire block width.
    void appendSolidsForBlock(int blockIdx, float y, float h, float viewL, float viewR,
                              std::vector<sf::FloatRect>& out) const {
        // --- Block horizontal bounds in world space ---
        const int   anchor = blockAnchor(blockIdx);
        const float blockL = anchor * m_cellWidth;
        const float blockR = (anchor + m_cellsPerBlock) * m_cellWidth;

        // Helper to emit a solid rect, clipped to the visible [viewL, viewR] range
        auto clipPush = [&](float L, float R) {
            const float left  = std::max(L, viewL);
            const float right = std::min(R, viewR);
            if (right > left)
                out.emplace_back(sf::FloatRect{{left, y}, {right - left, h}});
        };

        const std::optional<sf::FloatRect> gapOpt = computeGapForBlock(blockIdx, y, h);

        if (gapOpt) {
            const sf::FloatRect& gap  = *gapOpt;
            const float          gapL = geom::left(gap);
            const float          gapR = geom::left(gap) + geom::width(gap);

            // Left solid: from start of block to start of gap
            clipPush(blockL, gapL);
            // Right solid: from end of gap to end of block
            clipPush(gapR, blockR);
        } else {
            // Continuous ground: single solid segment for entire block
            clipPush(blockL, blockR);
        }
    }

  private:
    float m_bandHeightRatio = 0.05f;
    float m_cellWidth       = 220.f;
    int   m_cellsPerBlock   = 5;
    bool  m_hasGaps         = true;

    GapPatternFn m_gapPattern;

    // Aggregated AABBs that represent all solid ground segments under the current view
    MultiRectCollider m_collider;
};

