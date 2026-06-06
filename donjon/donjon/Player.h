#pragma once
#include "Actor.h"
#include "MovementComponent.h"
#include "SkeletalMeshComponent.h"
#include <string>
#include "CameraManager.h"

class Player : public Actor {
public:
    Player(const sf::Vector2f& spawnPosition = { 0.0f, 0.0f }) : Actor(spawnPosition) {
        _movement = addComponent<MovementComponent>();
        auto playerCamera = addComponent<CameraComponent>(sf::Vector2f(1920.f, 1080.f));
        CameraManager::getInstance().setActiveCamera(playerCamera);

        auto col = addComponent<BoxColliderComponent>(sf::Vector2f(200.f, 300.f));
        col->setCollisionObjectType(ECollisionChannel::Pawn);

        col->setCollisionResponse(ECollisionChannel::WorldStatic, ECollisionResponse::Block);
        col->setCollisionResponse(ECollisionChannel::WorldDynamic, ECollisionResponse::Block);
        col->setCollisionResponse(ECollisionChannel::Pawn, ECollisionResponse::Block);
        col->setCollisionResponse(ECollisionChannel::Trigger, ECollisionResponse::Overlap);

        auto animMesh = addComponent<SkeletalMeshComponent>("../assets/player.png");
        animMesh->addAnimation("Idle", AnimationData::detectFlipbook("../assets/player.png", 0.8f, true));
        animMesh->addAnimation("Walk", AnimationData::detectFlipbook("../assets/player_run.png", 0.8f, true));
        auto t = this->getComponent<TransformComponent>();
        t->setScale({ 0.6f,0.6f });

        _skeletalMesh = animMesh;
    }

    void tick(float deltaTime) override {
        sf::Vector2f direction{ 0.0f, 0.0f };

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z)) direction.y -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) direction.y += 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) direction.x -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) direction.x += 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P)) {
            CameraManager::getInstance().shake(1, 10);
        }

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
    float _speed{ 600.0f };
};