#pragma once

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

struct AnimationClip {
    std::string              name;
    const sf::Texture*       texture = nullptr;
    std::vector<sf::IntRect> frameRects; // Rects defining each frame in the animation
    float                    fps     = 8.f;
    bool                     looping = true;
};

class SpriteAnimator {
  public:
    explicit SpriteAnimator(sf::Sprite& sprite);

    void addClip(AnimationClip clip);

    // Start or switch to a clip. If non-looping, onCompleteOnce fires when the
    // clip finishes.
    void playClip(const std::string& clipName, std::function<void()> onCompleteOnce = {});

    void requestRestart();

    bool isPlayingClip(const std::string& clipName) const;

    // Helper: ensure a clip is playing without restarting if already active
    void ensureClip(const std::string& clipName);

    void update(float dt);

  private:
    sf::Sprite&                                    m_sprite;
    std::unordered_map<std::string, AnimationClip> m_clips;

    std::string           m_activeClipName;
    const AnimationClip*  m_activeClip = nullptr;
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
