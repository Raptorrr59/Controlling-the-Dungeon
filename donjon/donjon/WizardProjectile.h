#pragma once
#include "Actor.h"
#include "MovementComponent.h"
#include "BoxColliderComponent.h"
#include "ShapeComponent.h"
#include "World.h"
#include <cmath>

class WizardProjectile : public Actor {
public:
    WizardProjectile(const sf::Vector2f& spawnPosition, const sf::Vector2f& direction)
        : Actor(spawnPosition), _direction(direction) {
        
        _movement = addComponent<MovementComponent>();
        
        auto shape = addComponent<ShapeComponent>();
        shape->setShape(sf::Vector2f(20.0f, 20.0f), sf::Color::Yellow);

        auto col = addComponent<BoxColliderComponent>(sf::Vector2f(20.0f, 20.0f));
        col->setCollisionObjectType(ECollisionChannel::Projectile);
        
        col->setCollisionResponse(ECollisionChannel::WorldStatic, ECollisionResponse::Block);
        col->setCollisionResponse(ECollisionChannel::Pawn, ECollisionResponse::Overlap);
        col->setCollisionResponse(ECollisionChannel::Trigger, ECollisionResponse::Overlap);
    }

    void tick(float deltaTime) override {
        if (auto movement = _movement.lock()) {
            movement->setVelocity(_direction * _speed);
        }

        _lifeTimer -= deltaTime;
        if (_lifeTimer <= 0.0f) {
            destroy();
        }

        auto col = getComponent<BoxColliderComponent>();
        if (col) {
            sf::Vector2f repulsion;
            if (World::getWorld().checkMoveCollision(*col, repulsion)) {
                destroy(); 
            }
        }

        Actor::tick(deltaTime);
    }

private:
    std::weak_ptr<MovementComponent> _movement;
    sf::Vector2f _direction;
    float _speed{ 900.0f };
    float _lifeTimer{ 2.5f }; 
};
