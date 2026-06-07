#pragma once
#include "Actor.h"
#include "MovementComponent.h"
#include "BoxColliderComponent.h"
#include "ShapeComponent.h"
#include "World.h"
#include "Possessable.h"
#include <cmath>
#include <functional>

class Possessor;

class PossessionOrb : public Actor {
public:
    PossessionOrb(const sf::Vector2f& spawnPosition, const sf::Vector2f& direction, Possessor* ownerPossessor)
        : Actor(spawnPosition), _direction(direction), _ownerPossessor(ownerPossessor) {
        
        _movement = addComponent<MovementComponent>();
        
        auto shape = addComponent<ShapeComponent>();
        shape->setShape(sf::Vector2f(24.0f, 24.0f), sf::Color::Cyan);

        auto col = addComponent<BoxColliderComponent>(sf::Vector2f(24.0f, 24.0f));
        col->setCollisionObjectType(ECollisionChannel::Projectile);
        col->setCollisionResponse(ECollisionChannel::WorldStatic, ECollisionResponse::Block);
        col->setCollisionResponse(ECollisionChannel::Pawn, ECollisionResponse::Overlap);
        col->setCollisionResponse(ECollisionChannel::Trigger, ECollisionResponse::Overlap);

        col->onComponentBeginOverlap = [this](Actor* other) {
            if (other) {
                if (auto possessable = dynamic_cast<Possessable*>(other)) {
                    if (!possessable->isPossessed()) {
                        handlePossessTarget(possessable);
                        destroy();
                    }
                }
            }
        };
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
    void handlePossessTarget(Possessable* target);

    std::weak_ptr<MovementComponent> _movement;
    sf::Vector2f _direction;
    Possessor* _ownerPossessor;
    float _speed{ 1200.0f };
    float _lifeTimer{ 1.5f };
};
