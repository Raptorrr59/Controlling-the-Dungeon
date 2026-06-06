#pragma once
#include "ActorComponent.h"
#include <SFML/Graphics.hpp>

class RenderComponent : public ActorComponent, public sf::Drawable {
public:
    RenderComponent(Actor* owner) : ActorComponent(owner) {}
    virtual ~RenderComponent() = default;

protected:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override = 0;
};