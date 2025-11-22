#include "ResourceManager.h"

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

const sf::Font& ResourceManager::getFont(const std::string& filename) {
    if (auto it = m_loadedFonts.find(filename); it != m_loadedFonts.end())
        return it->second;

    auto [it, inserted] = m_loadedFonts.emplace(filename, sf::Font{});
    if (!inserted) {
        throw std::runtime_error("ResourceManager::getFont: failed to emplace font entry for '" +
                                 filename + "'");
    }

    sf::Font&  font     = it->second;
    const auto fullPath = getAssetPath(filename);
    if (!font.openFromFile(fullPath)) {
        throw std::runtime_error("ResourceManager::getFont: could not load font from '" +
                                 fullPath.string() + "'");
    }

    return font;
}

sf::Texture& ResourceManager::getTexture(const std::string& filename) {
    if (auto it = m_loadedTextures.find(filename); it != m_loadedTextures.end())
        return it->second;

    auto [it, inserted] = m_loadedTextures.emplace(filename, sf::Texture{});
    if (!inserted) {
        throw std::runtime_error(
            "ResourceManager::getTexture: failed to emplace texture entry for '" + filename + "'");
    }

    sf::Texture& texture  = it->second;
    const auto   fullPath = getAssetPath(filename);
    if (!texture.loadFromFile(fullPath)) {
        throw std::runtime_error("ResourceManager::getTexture: could not load texture from '" +
                                 fullPath.string() + "'");
    }

    return texture;
}

const sf::SoundBuffer& ResourceManager::getSoundBuffer(const std::string& filename) {
    if (auto it = m_loadedSoundBuffers.find(filename); it != m_loadedSoundBuffers.end())
        return it->second;

    auto [it, inserted] = m_loadedSoundBuffers.emplace(filename, sf::SoundBuffer{});
    if (!inserted) {
        throw std::runtime_error(
            "ResourceManager::getSoundBuffer: failed to emplace sound buffer entry for '" +
            filename + "'");
    }

    sf::SoundBuffer& buffer   = it->second;
    const auto       fullPath = getAssetPath(filename);
    if (!buffer.loadFromFile(fullPath)) {
        throw std::runtime_error(
            "ResourceManager::getSoundBuffer: could not load sound buffer from '" +
            fullPath.string() + "'");
    }

    return buffer;
}

std::filesystem::path ResourceManager::getAssetPath(const std::string& filename) {
    return std::filesystem::path(m_assetPath) / filename;
}
