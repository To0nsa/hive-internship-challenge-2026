#include "Animation.h"

#include <algorithm>
#include <utility>

// SpriteAnimator implementation
SpriteAnimator::SpriteAnimator(sf::Sprite& sprite) : m_sprite(sprite) {}

void SpriteAnimator::addClip(AnimationClip clip) { m_clips.emplace(clip.name, std::move(clip)); }

void SpriteAnimator::playClip(const std::string& clipName, std::function<void()> onCompleteOnce) {
    if (m_activeClipName == clipName && !m_restartRequested)
        return;
    auto it = m_clips.find(clipName);
    if (it == m_clips.end())
        return;

    m_activeClipName   = clipName;
    m_activeClip       = &it->second;
    m_onCompleteOnce   = std::move(onCompleteOnce);
    m_timeAccumulator  = 0.f;
    m_frameIndex       = 0;
    m_restartRequested = false;
    if (m_activeClip->texture)
        m_sprite.setTexture(*m_activeClip->texture);
    applyFrameRect();
}

void SpriteAnimator::requestRestart() { m_restartRequested = true; }

bool SpriteAnimator::isPlayingClip(const std::string& clipName) const {
    return m_activeClipName == clipName;
}

void SpriteAnimator::ensureClip(const std::string& clipName) {
    if (!isPlayingClip(clipName))
        playClip(clipName);
}

void SpriteAnimator::update(float dt) {
    if (!m_activeClip)
        return;
    if (m_activeClip->frameRects.empty() || m_activeClip->fps <= 0.f)
        return;

    m_timeAccumulator += dt;
    const float frameDuration = 1.f / m_activeClip->fps;

    while (m_timeAccumulator >= frameDuration) {
        m_timeAccumulator -= frameDuration;
        advanceFrame(*m_activeClip);
    }
}

void SpriteAnimator::advanceFrame(const AnimationClip& clip) {
    if (m_frameIndex + 1 < clip.frameRects.size()) {
        ++m_frameIndex;
        applyFrameRect();
        return;
    }

    if (clip.looping) {
        m_frameIndex = 0;
        applyFrameRect();
    } else {
        // Hold on the last frame and fire completion once.
        if (m_onCompleteOnce) {
            auto cb          = std::move(m_onCompleteOnce);
            m_onCompleteOnce = nullptr;
            cb();
        }
    }
}

void SpriteAnimator::applyFrameRect() {
    if (!m_activeClip || m_activeClip->frameRects.empty())
        return;
    m_sprite.setTextureRect(m_activeClip->frameRects[m_frameIndex]);
}

// Animation namespace implementation
namespace Animation {

    AnimationClip makeClipFromRow(const std::string& name, const sf::Texture& texture,
                                  sf::Vector2i frameSize, int frameCount, float fps, bool looping) {
        AnimationClip clip;
        clip.name    = name;
        clip.texture = &texture;
        clip.fps     = fps;
        clip.looping = looping;

        clip.frameRects.reserve(frameCount);
        for (int i = 0; i < frameCount; ++i) {
            clip.frameRects.emplace_back(
                sf::IntRect{{i * frameSize.x, 0}, {frameSize.x, frameSize.y}});
        }
        return clip;
    }

    AnimationClip makeClipFromSheet(const std::string& name, const sf::Texture& texture,
                                    sf::Vector2i frameSize, sf::Vector2i firstCell,
                                    sf::Vector2i lastCell, float fps, bool looping) {
        AnimationClip clip;
        clip.name    = name;
        clip.texture = &texture;
        clip.fps     = fps;
        clip.looping = looping;

        // Grid metrics
        const sf::Vector2u texSz = texture.getSize();
        const int          cols  = (frameSize.x > 0) ? static_cast<int>(texSz.x) / frameSize.x : 0;
        const int          rows  = (frameSize.y > 0) ? static_cast<int>(texSz.y) / frameSize.y : 0;
        if (cols <= 0 || rows <= 0)
            return clip;

        auto toIndex = [cols](sf::Vector2i cell) { return cell.y * cols + cell.x; };

        // Clamp to grid and ensure start <= end
        firstCell.x = std::clamp(firstCell.x, 0, cols - 1);
        firstCell.y = std::clamp(firstCell.y, 0, rows - 1);
        lastCell.x  = std::clamp(lastCell.x, 0, cols - 1);
        lastCell.y  = std::clamp(lastCell.y, 0, rows - 1);

        int start = toIndex(firstCell);
        int end   = toIndex(lastCell);
        if (start > end)
            std::swap(start, end);

        clip.frameRects.reserve(static_cast<std::size_t>(end - start + 1));
        for (int i = start; i <= end; ++i) {
            const int c = i % cols;
            const int r = i / cols;
            clip.frameRects.emplace_back(
                sf::IntRect{{c * frameSize.x, r * frameSize.y}, {frameSize.x, frameSize.y}});
        }
        return clip;
    }

} // namespace Animation
