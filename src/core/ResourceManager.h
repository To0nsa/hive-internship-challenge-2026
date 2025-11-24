#pragma once

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

class ResourceManager {
  public:
    static void                   init(std::string executablePath);
    static const sf::Font&        getFont(std::string_view filename);
    static sf::Texture&           getTexture(std::string_view filename);
    static sf::Texture&           getRepeatedTexture(std::string_view filename);
    static const sf::SoundBuffer& getSoundBuffer(std::string_view filename);

  private:
    static inline std::string                                      m_assetPath;
    static inline std::unordered_map<std::string, sf::Font>        m_loadedFonts;
    static inline std::unordered_map<std::string, sf::Texture>     m_loadedTextures;
    static inline std::unordered_map<std::string, sf::SoundBuffer> m_loadedSoundBuffers;

    static std::filesystem::path getAssetPath(std::string_view filename);
};
