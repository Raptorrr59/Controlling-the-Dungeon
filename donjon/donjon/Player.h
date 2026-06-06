#pragma once
#include "Actor.h"
#include "MovementComponent.h"
#include "SkeletalMeshComponent.h"
#include <string>

class Player : public Actor {
public:
    Player(const sf::Vector2f& spawnPosition = { 0.0f, 0.0f }) : Actor(spawnPosition) {
        _movement = addComponent<MovementComponent>();

        auto t = this->getComponent<TransformComponent>();
        if (t) {
            std::cout << "t exist" << std::endl;
        }

        auto animMesh = addComponent<SkeletalMeshComponent>("../assets/player.png");

        animMesh->addAnimation("Idle", AnimationData::detectFlipbook("../assets/player.png", 0.8f, true));
        animMesh->addAnimation("Walk", AnimationData::detectFlipbook("../assets/player_run.png", 0.8f, true));

        _skeletalMesh = animMesh;
    }

    void tick(float deltaTime) override {
        sf::Vector2f direction{ 0.0f, 0.0f };

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z)) direction.y -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) direction.y += 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) direction.x -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) direction.x += 1.0f;

        if (auto mesh = _skeletalMesh.lock()) {
            if (direction.x != 0.0f || direction.y != 0.0f) {
                auto t = this->getComponent<TransformComponent>();
                if (direction.x < 0.0f) {
                    t->setScale({ -1.0f,1.0f });
                }
                else {
                    t->setScale({ 1.0f,1.0f });
                }
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