#pragma once
#include "Actor.h"
#include "MovementComponent.h"
#include "SkeletalMeshComponent.h"
#include <string>

class Wizard : public Actor {
public:
    Wizard(const sf::Vector2f& spawnPosition = { 0.0f, 0.0f }) : Actor(spawnPosition) {
        _movement = addComponent<MovementComponent>();

        auto col = addComponent<BoxColliderComponent>(sf::Vector2f(52.f, 83.f));
        col->setCollisionObjectType(ECollisionChannel::Pawn);

        col->setCollisionResponse(ECollisionChannel::WorldStatic, ECollisionResponse::Block);
        col->setCollisionResponse(ECollisionChannel::WorldDynamic, ECollisionResponse::Block);
        col->setCollisionResponse(ECollisionChannel::Pawn, ECollisionResponse::Block);
        col->setCollisionResponse(ECollisionChannel::Trigger, ECollisionResponse::Overlap);

        auto animMesh = addComponent<SkeletalMeshComponent>("../assets/wizard_idle.png");
        animMesh->addAnimation("Idle", AnimationData::detectFlipbook("../assets/wizard_idle.png", 1.6f, true));
        animMesh->addAnimation("Walk", AnimationData::detectFlipbook("../assets/wizard_run.png", 0.8f, true));
        auto t = this->getComponent<TransformComponent>();
        //t->setScale({ 0.6f,0.6f });

        _skeletalMesh = animMesh;
    }

    void tick(float deltaTime) override {
        sf::Vector2f direction{ 0.0f, 0.0f };

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) direction.y -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) direction.y += 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) direction.x -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) direction.x += 1.0f;

        if (auto mesh = _skeletalMesh.lock()) {
            if (direction.x != 0.0f || direction.y != 0.0f) {
                
                mesh->playAnimation("Walk");
            }
            else {
                mesh->playAnimation("Idle");
            }
        }

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