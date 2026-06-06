#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>

class SmartTextureLoader {
public:
    static const sf::Texture& getTexture(const std::string& filePath) {
        auto it = _textures.find(filePath);
        if (it != _textures.end()) {
            return *it->second;
        }

        auto texture = std::make_unique<sf::Texture>();
        if (!texture->loadFromFile(filePath)) {
            throw std::runtime_error("cannot load : " + filePath);
        }

        const sf::Texture& textureRef = *texture;
        _textures[filePath] = std::move(texture);

        return textureRef;
    }

private:
    inline static std::unordered_map<std::string, std::unique_ptr<sf::Texture>> _textures;
};