#pragma once

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <filesystem>
#include <string>
#include <unordered_map>

class ResourceManager {
  public:
    static void                   init(std::string executablePath);
    static const sf::Font*        getFont(const std::string& filename);
    static const sf::Texture*     getTexture(const std::string& filename);
    static const sf::SoundBuffer* getSoundBuffer(const std::string& filename);

  private:
    static inline std::string                                      m_assetPath;
    static inline std::unordered_map<std::string, sf::Font>        m_loadedFonts;
    static inline std::unordered_map<std::string, sf::Texture>     m_loadedTextures;
    static inline std::unordered_map<std::string, sf::SoundBuffer> m_loadedSoundBuffers;

    static std::filesystem::path getAssetPath(const std::string& filename);
};
