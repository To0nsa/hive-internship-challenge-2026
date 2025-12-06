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

class GroundBand {
  public:
    explicit GroundBand(const GroundStreamConfig& cfg)
        : m_bandHeightRatio(cfg.bandHeightRatio), m_cellWidth(cfg.cellWidth),
          m_cellsPerBlock(cfg.cellsPerBlock), m_gapPattern(cfg.gapPattern) {}

    // Rebuild the set of collidable ground rectangles for the current camera view.
    void updateForView(const sf::View& view) {
        const ViewBandInfo info = computeViewBandInfo(view);

        std::vector<sf::FloatRect> solids;
        const int                  blockCount = std::max(0, info.lastBlock - info.firstBlock + 1);
        // Worst case: 2 solids per block (left + right of gap).
        solids.reserve(blockCount * 2);

        for (int block = info.firstBlock; block <= info.lastBlock; ++block)
            appendSolidsForBlock(block, info, solids);

        m_collider.setRectColliders(std::move(solids));
    }

    // Access to ground collider used by physics / actors
    const MultiRectCollider& getCollider() const { return m_collider; }

    // Top Y of the collidable band for a given view.
    float getTopYForView(const sf::View& view) const { return computeViewBandInfo(view).y; }

    // Computes gap rectangles for the given view in world space.
    void gapsForView(const sf::View& view, std::vector<GroundGap>& outGaps) const {
        outGaps.clear();

        const ViewBandInfo info = computeViewBandInfo(view);

        for (int block = info.firstBlock; block <= info.lastBlock; ++block) {
            const std::optional<sf::FloatRect> gapOpt =
                computeGapForBlock(block, info.y, info.height);
            if (!gapOpt)
                continue;

            const sf::FloatRect& gap              = *gapOpt;
            const float          gapL             = geom::left(gap);
            const float          gapR             = gapL + geom::width(gap);
            const bool           gapIsLeftOfView  = gapR < info.viewLeft;
            const bool           gapIsRightOfView = gapL > info.viewRight;
            if (gapIsLeftOfView || gapIsRightOfView)
                continue;

            GroundGap groundGap;
            groundGap.rect       = gap;
            groundGap.blockIndex = block;
            outGaps.push_back(groundGap);
        }
    }

  private:
    struct ViewBandInfo {
        float y          = 0.f;
        float height     = 0.f;
        float viewLeft   = 0.f;
        float viewRight  = 0.f;
        int   firstBlock = 0;
        int   lastBlock  = -1;
    };

    ViewBandInfo computeViewBandInfo(const sf::View& view) const {
        ViewBandInfo info;

        const sf::Vector2f viewSize   = view.getSize();
        const sf::Vector2f viewCenter = view.getCenter();

        info.viewLeft  = viewCenter.x - 0.5f * viewSize.x;
        info.viewRight = info.viewLeft + viewSize.x;

        info.height = viewSize.y * m_bandHeightRatio;
        info.y      = (viewCenter.y - 0.5f * viewSize.y) + (viewSize.y - info.height);

        const int firstCell = static_cast<int>(std::floor(info.viewLeft / m_cellWidth));
        const int lastCell  = static_cast<int>(std::ceil(info.viewRight / m_cellWidth));

        info.firstBlock = math::floorDivInt(firstCell, m_cellsPerBlock) - 1;
        info.lastBlock  = math::floorDivInt(lastCell, m_cellsPerBlock) + 1;

        return info;
    }

    // Returns the world-space rectangle of the gap within this block, if any.
    // - Gap width/position is fully decided by m_gapPattern
    // - Pattern can decide per-block to have or not have a gap (nullopt).
    std::optional<sf::FloatRect> computeGapForBlock(int blockIdx, float y, float h) const {
        if (m_cellsPerBlock <= 1)
            return std::nullopt;

        // By design, m_gapPattern is always set (defaults to "no gaps").
        return m_gapPattern(blockIdx, y, h, m_cellWidth, m_cellsPerBlock);
    }

    // Appends solid segments of a block, clipped to [viewLeft, viewRight].
    void appendSolidsForBlock(int blockIdx, const ViewBandInfo& info,
                              std::vector<sf::FloatRect>& out) const {
        const int   anchor = blockIdx * m_cellsPerBlock;
        const float blockL = anchor * m_cellWidth;
        const float blockR = (anchor + m_cellsPerBlock) * m_cellWidth;

        auto clipPush = [&](float L, float R) {
            const float left  = std::max(L, info.viewLeft);
            const float right = std::min(R, info.viewRight);
            if (right > left)
                out.emplace_back(sf::FloatRect{{left, info.y}, {right - left, info.height}});
        };

        const std::optional<sf::FloatRect> gapOpt =
            computeGapForBlock(blockIdx, info.y, info.height);

        if (gapOpt) {
            const sf::FloatRect& gap  = *gapOpt;
            const float          gapL = geom::left(gap);
            const float          gapR = gapL + geom::width(gap);

            clipPush(blockL, gapL); // left solid
            clipPush(gapR, blockR); // right solid
        } else {
            clipPush(blockL, blockR); // full block solid
        }
    }

  private:
    float        m_bandHeightRatio;
    float        m_cellWidth;
    int          m_cellsPerBlock;
    GapPatternFn m_gapPattern;

    MultiRectCollider m_collider;
};
