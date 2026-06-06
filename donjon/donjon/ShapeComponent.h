#pragma once
#include "RenderComponent.h"
#include "Actor.h"

class ShapeComponent : public RenderComponent {
public:
    ShapeComponent(Actor* owner) : RenderComponent(owner) {}

    void setShape(const sf::Vector2f& size, const sf::Color& color) {
        _shape.setSize(size);
        _shape.setFillColor(color);
    }

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        target.draw(_shape, states);
    }

private:
    sf::RectangleShape _shape;
};