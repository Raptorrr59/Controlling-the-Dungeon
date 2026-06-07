#pragma once
#include "Actor.h"
#include "BoxColliderComponent.h"
#include "ShapeComponent.h"

class SolidBlock : public Actor {
public:
    SolidBlock(const sf::Vector2f& spawnPosition, const sf::Vector2f& size = { 80.0f, 80.0f }, const sf::Color& color = sf::Color(45, 50, 65))
        : Actor(spawnPosition), _size(size) {
        
        _shape = addComponent<ShapeComponent>();
        _shape->setShape(_size, color);

        auto col = addComponent<BoxColliderComponent>(_size);
        col->setCollisionObjectType(ECollisionChannel::WorldStatic);
        
        col->setCollisionResponse(ECollisionChannel::Pawn, ECollisionResponse::Block);
        col->setCollisionResponse(ECollisionChannel::Projectile, ECollisionResponse::Block);
    }

private:
    sf::Vector2f _size;
    std::shared_ptr<ShapeComponent> _shape;
};
