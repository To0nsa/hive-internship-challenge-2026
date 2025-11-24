#include "core/ResourceManager.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

void ResourceManager::init(std::string executablePath) {
    std::replace(executablePath.begin(), executablePath.end(), '\\', '/');
    size_t lastSlashIndex = executablePath.find_last_of('/');
    if (lastSlashIndex != std::string::npos)
        m_assetPath = executablePath.substr(0, lastSlashIndex + 1);
    m_assetPath += "assets/";
}

const sf::Font& ResourceManager::getFont(std::string_view filename) {
    const std::string key(filename);

    if (auto it = m_loadedFonts.find(key); it != m_loadedFonts.end())
        return it->second;

    auto [it, inserted] = m_loadedFonts.emplace(key, sf::Font{});
    if (!inserted) {
        throw std::runtime_error("ResourceManager::getFont: failed to emplace font entry for '" +
                                 key + "'");
    }

    sf::Font&  font     = it->second;
    const auto fullPath = getAssetPath(key);
    if (!font.openFromFile(fullPath)) {
        throw std::runtime_error("ResourceManager::getFont: could not load font from '" + key +
                                 "' at '" + fullPath.string() + "'");
    }

    return font;
}

sf::Texture& ResourceManager::getTexture(std::string_view filename) {
    const std::string key(filename);

    if (auto it = m_loadedTextures.find(key); it != m_loadedTextures.end())
        return it->second;

    auto [it, inserted] = m_loadedTextures.emplace(key, sf::Texture{});
    if (!inserted) {
        throw std::runtime_error(
            "ResourceManager::getTexture: failed to emplace texture entry for '" + key + "'");
    }

    sf::Texture& texture  = it->second;
    const auto   fullPath = getAssetPath(key);
    if (!texture.loadFromFile(fullPath)) {
        throw std::runtime_error("ResourceManager::getTexture: could not load texture from '" +
                                 key + "' at '" + fullPath.string() + "'");
    }

    return texture;
}

sf::Texture& ResourceManager::getRepeatedTexture(std::string_view filename) {
    sf::Texture& texture = getTexture(filename);

    if (texture.isRepeated())
        return texture;

    texture.setRepeated(true);
    return texture;
}

const sf::SoundBuffer& ResourceManager::getSoundBuffer(std::string_view filename) {
    const std::string key(filename);

    if (auto it = m_loadedSoundBuffers.find(key); it != m_loadedSoundBuffers.end())
        return it->second;

    auto [it, inserted] = m_loadedSoundBuffers.emplace(key, sf::SoundBuffer{});
    if (!inserted) {
        throw std::runtime_error(
            "ResourceManager::getSoundBuffer: failed to emplace sound buffer entry for '" + key +
            "'");
    }

    sf::SoundBuffer& buffer   = it->second;
    const auto       fullPath = getAssetPath(key);
    if (!buffer.loadFromFile(fullPath)) {
        throw std::runtime_error(
            "ResourceManager::getSoundBuffer: could not load sound buffer from '" + key + "' at '" +
            fullPath.string() + "'");
    }

    return buffer;
}

std::filesystem::path ResourceManager::getAssetPath(std::string_view filename) {
    return std::filesystem::path(m_assetPath) / std::string(filename);
}
