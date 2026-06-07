#pragma once
#include "Possessable.h"
#include "PlatformerMovementComponent.h"
#include "SkeletalMeshComponent.h"
#include "CameraManager.h"
#include <cmath>

class PossessableKnight : public Possessable {
public:
    PossessableKnight(const sf::Vector2f& spawnPosition = { 0.0f, 0.0f }) : Possessable(spawnPosition) {
        _movement = addComponent<PlatformerMovementComponent>();
        
        _camera = addComponent<CameraComponent>(sf::Vector2f(1920.f, 1080.f));
        _camera->setZoom(1.2f); // Zoom out camera view

        // Raw player dimensions scaled by 0.35f (collider width 63, height 108.5), centered
        auto col = addComponent<BoxColliderComponent>(sf::Vector2f(180.f, 310.f), true);
        col->setCollisionObjectType(ECollisionChannel::Pawn);

        col->setCollisionResponse(ECollisionChannel::WorldStatic, ECollisionResponse::Block);
        col->setCollisionResponse(ECollisionChannel::WorldDynamic, ECollisionResponse::Block);
        col->setCollisionResponse(ECollisionChannel::Pawn, ECollisionResponse::Ignore);
        col->setCollisionResponse(ECollisionChannel::Trigger, ECollisionResponse::Overlap);

        auto animMesh = addComponent<SkeletalMeshComponent>("../assets/player.png");
        animMesh->addAnimation("Idle", AnimationData::detectFlipbook("../assets/player.png", 0.8f, true));
        animMesh->addAnimation("Walk", AnimationData::detectFlipbook("../assets/player_run.png", 0.8f, true));
        
        auto t = this->getComponent<TransformComponent>();
        t->setScale({ 0.35f, 0.35f }); 

        _skeletalMesh = animMesh;
    }

    void onPossess() override {
        Possessable::onPossess();
        CameraManager::getInstance().setActiveCamera(_camera);
    }

    void tick(float deltaTime) override {
        sf::Vector2f velocity = { 0.0f, 0.0f };
        if (auto movement = _movement.lock()) {
            velocity = movement->getVelocity();
            velocity.x = 0.0f; 

            if (_bIsPossessed) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q) || 
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || 
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
                    velocity.x -= _speed;
                    auto t = getComponent<TransformComponent>();
                    if (t && t->getScale().x > 0.0f) {
                        t->setScale({ -0.35f, 0.35f });
                    }
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || 
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
                    velocity.x += _speed;
                    auto t = getComponent<TransformComponent>();
                    if (t && t->getScale().x < 0.0f) {
                        t->setScale({ 0.35f, 0.35f });
                    }
                }

                if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) || 
                     sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z) || 
                     sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || 
                     sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) && movement->isOnGround()) {
                    velocity.y = -_jumpForce;
                }
            }

            movement->setVelocity(velocity);
        }

        if (auto mesh = _skeletalMesh.lock()) {
            if (std::abs(velocity.x) > 1.0f) {
                mesh->playAnimation("Walk");
            } else {
                mesh->playAnimation("Idle");
            }
        }

        Possessable::tick(deltaTime);
    }

private:
    std::weak_ptr<PlatformerMovementComponent> _movement;
    std::weak_ptr<SkeletalMeshComponent> _skeletalMesh;
    std::shared_ptr<CameraComponent> _camera;
    float _speed{ 450.0f };
    float _jumpForce{ 900.0f };
};
