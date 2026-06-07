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
        sf::Vector2u size = _sprite.getTexture().getSize();
        _sprite.setOrigin({ size.x / 2.0f, size.y / 2.0f });
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