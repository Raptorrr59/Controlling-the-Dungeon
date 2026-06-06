#pragma once
#include "ActorComponent.h"
#include <SFML/Graphics.hpp>

class TransformComponent : public ActorComponent {
public:
    TransformComponent(Actor* owner, const sf::Vector2f& position = { 0.0f, 0.0f })
        : ActorComponent(owner), _position(position), _scale({ 1.0f, 1.0f }) {
    }

    void setPosition(const sf::Vector2f& position) { _position = position; }
    const sf::Vector2f& getPosition() const { return _position; }

    void setScale(const sf::Vector2f& scale) { _scale = scale; }
    const sf::Vector2f& getScale() const { return _scale; }

private:
    sf::Vector2f _position;
    sf::Vector2f _scale;
};