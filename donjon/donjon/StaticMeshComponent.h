#pragma once
#include "RenderComponent.h"
#include "SmartTextureLoader.h"
#include <string>

class StaticMeshComponent : public RenderComponent {
public:
    StaticMeshComponent(Actor* owner, const std::string& texturePath)
        : RenderComponent(owner),
        _sprite(SmartTextureLoader::getTexture(texturePath))
    {
    }

    void setTextureRect(const sf::IntRect& rect) {
        _sprite.setTextureRect(rect);
    }

    void setColor(const sf::Color& color) {
        _sprite.setColor(color);
    }

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        target.draw(_sprite, states);
    }

private:
    sf::Sprite _sprite;
};