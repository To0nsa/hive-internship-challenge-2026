#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstdint>
#include <functional>
#include <limits>
#include <string>
#include <vector>

struct AnimationClip {
    struct Frame {
        sf::IntRect rect{};
    };

    std::string        name;
    const sf::Texture* texture = nullptr;
    std::vector<Frame> frames;
    float              fps     = 8.f;
    bool               looping = true;
};

class SpriteAnimator {
  public:
    using ClipId                         = std::size_t;
    static constexpr ClipId kInvalidClip = std::numeric_limits<ClipId>::max();

    explicit SpriteAnimator(sf::Sprite& sprite);

    ClipId addClip(AnimationClip clip);

    // Start or switch to a clip. If non-looping, onCompleteOnce fires when the
    // clip finishes.
    void playClip(ClipId clipId, std::function<void()> onCompleteOnce = {});

    // Request a restart of the current clip at the next update
    void requestRestart();

    void update(float dt);

  private:
    sf::Sprite&                m_sprite;
    std::vector<AnimationClip> m_clips;

    ClipId                m_activeClipId = kInvalidClip;
    std::function<void()> m_onCompleteOnce;

    float       m_timeAccumulator  = 0.f;
    std::size_t m_frameIndex       = 0;
    bool        m_restartRequested = false;

    void advanceFrame(const AnimationClip& clip);
    void applyFrameRect();
};

namespace Animation {

    AnimationClip makeClipFromRow(const std::string& name, const sf::Texture& texture,
                                  sf::Vector2i frameSize, int frameCount, float fps, bool looping);

    AnimationClip makeClipFromSheet(const std::string& name, const sf::Texture& texture,
                                    sf::Vector2i frameSize, sf::Vector2i firstCell,
                                    sf::Vector2i lastCell, float fps, bool looping);

} // namespace Animation
