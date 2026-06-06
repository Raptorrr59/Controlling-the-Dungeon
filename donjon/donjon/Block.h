#pragma once
#include "Actor.h"
#include "MovementComponent.h"
#include "SkeletalMeshComponent.h"
#include "BoxColliderComponent.h"
#include <string>

class Block : public Actor {
public:
    Block(const sf::Vector2f& spawnPosition = { 0.0f, 0.0f }) : Actor(spawnPosition) {
        _movement = addComponent<MovementComponent>();

        auto col = addComponent<BoxColliderComponent>(sf::Vector2f(3000.0f, 300.f));
        col->setCollisionObjectType(ECollisionChannel::Pawn);

        col->setCollisionResponse(ECollisionChannel::WorldStatic, ECollisionResponse::Block);
        col->setCollisionResponse(ECollisionChannel::WorldDynamic, ECollisionResponse::Block);
        col->setCollisionResponse(ECollisionChannel::Pawn, ECollisionResponse::Block);
        col->setCollisionResponse(ECollisionChannel::Trigger, ECollisionResponse::Overlap);

        auto animMesh = addComponent<SkeletalMeshComponent>("../assets/player.png");

        _skeletalMesh = animMesh;
    }

    void tick(float deltaTime) override {
        sf::Vector2f direction{ 0.0f, 0.0f };

        if (auto movement = _movement.lock()) {
            movement->setVelocity(direction * _speed);
        }

        Actor::tick(deltaTime);
    }

private:
    std::weak_ptr<MovementComponent> _movement;
    std::weak_ptr<SkeletalMeshComponent> _skeletalMesh;
    float _speed{ 800.0f };
};