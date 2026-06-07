#pragma once
#include "Actor.h"
#include "MovementComponent.h"
#include "BoxColliderComponent.h"
#include "StaticMeshComponent.h"
#include "CameraManager.h"
#include "Possessable.h"
#include "PossessionOrb.h"
#include "Window.h"
#include "PossessableKnight.h"
#include "PossessableWizard.h"
#include <cmath>

class Possessor : public Actor {
public:
    Possessor(const sf::Vector2f& spawnPosition = { 0.0f, 0.0f }) : Actor(spawnPosition) {
        _movement = addComponent<MovementComponent>();
        
        _camera = addComponent<CameraComponent>(sf::Vector2f(1920.f, 1080.f));
        _camera->setZoom(1.4f); // Zoom out to see more of the level
        CameraManager::getInstance().setActiveCamera(_camera);

        // Raw size 1024x1024, scaled by 0.06f is 61.4x61.4 pixels, centered
        auto col = addComponent<BoxColliderComponent>(sf::Vector2f(1024.f, 1024.f), true);
        col->setCollisionObjectType(ECollisionChannel::Pawn);

        col->setCollisionResponse(ECollisionChannel::WorldStatic, ECollisionResponse::Block);
        col->setCollisionResponse(ECollisionChannel::WorldDynamic, ECollisionResponse::Block);
        col->setCollisionResponse(ECollisionChannel::Pawn, ECollisionResponse::Block);
        col->setCollisionResponse(ECollisionChannel::Trigger, ECollisionResponse::Overlap);

        auto mesh = addComponent<StaticMeshComponent>("../assets/ghost.png");
        auto t = this->getComponent<TransformComponent>();
        t->setScale({ 0.08f, 0.08f }); // Scale down the giant 1024x1024 texture
    }

    void possess(Possessable* target) {
        if (!target || target->isPossessed()) return;
        
        if (_possessedTarget) {
            _possessedTarget->onUnpossess();
        }
        
        _possessedTarget = target;
        _possessedTarget->onPossess();

        // Hide ghost
        getTransform()->setScale({ 0.0f, 0.0f });
        
        // Disable collision
        auto col = getComponent<BoxColliderComponent>();
        if (col) {
            col->setCollisionResponse(ECollisionChannel::WorldStatic, ECollisionResponse::Ignore);
            col->setCollisionResponse(ECollisionChannel::WorldDynamic, ECollisionResponse::Ignore);
            col->setCollisionResponse(ECollisionChannel::Pawn, ECollisionResponse::Ignore);
        }

        if (auto movement = _movement.lock()) {
            movement->setVelocity({ 0.0f, 0.0f });
        }

        CameraManager::getInstance().shake(0.2f, 10.0f);
    }

    void unpossess() {
        if (!_possessedTarget) return;

        auto targetPos = _possessedTarget->getTransform()->getPosition();
        _possessedTarget->onUnpossess();
        _possessedTarget = nullptr;

        // Show ghost and position it slightly above the target
        getTransform()->setScale({ 0.08f, 0.08f });
        getTransform()->setPosition(targetPos + sf::Vector2f(0.0f, -80.0f));

        // Re-enable collision
        auto col = getComponent<BoxColliderComponent>();
        if (col) {
            col->setCollisionResponse(ECollisionChannel::WorldStatic, ECollisionResponse::Block);
            col->setCollisionResponse(ECollisionChannel::WorldDynamic, ECollisionResponse::Block);
            col->setCollisionResponse(ECollisionChannel::Pawn, ECollisionResponse::Block);
        }

        // Reactivate camera
        CameraManager::getInstance().setActiveCamera(_camera);

        CameraManager::getInstance().shake(0.15f, 6.0f);
    }

    void tick(float deltaTime) override {
        if (_orbCooldownTimer > 0.0f) {
            _orbCooldownTimer -= deltaTime;
        }

        if (_possessedTarget) {
            if (auto targetTransform = _possessedTarget->getTransform()) {
                getTransform()->setPosition(targetTransform->getPosition());
            }

            // Unpossess check (Right Click or Shift keys)
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) || 
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift)) {
                unpossess();
                Actor::tick(deltaTime);
                return;
            }

            // Possession Swap/Shoot check while possessing
            bool bShouldShootOrb = false;
            if (dynamic_cast<PossessableKnight*>(_possessedTarget)) {
                // Knight: Left Click, E, or F
                bShouldShootOrb = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) || 
                                  sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E) || 
                                  sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F);
            } else if (dynamic_cast<PossessableWizard*>(_possessedTarget)) {
                // Wizard: F or Middle Click
                bShouldShootOrb = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F) || 
                                  sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle);
            }

            if (bShouldShootOrb && _orbCooldownTimer <= 0.0f) {
                _orbCooldownTimer = _orbCooldown;
                shootOrb();
            }

            Actor::tick(deltaTime);
            return;
        }

        // Flight controls for Ghost
        sf::Vector2f direction{ 0.0f, 0.0f };
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z) || 
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || 
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) direction.y -= 1.0f;
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || 
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) direction.y += 1.0f;
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q) || 
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || 
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) direction.x -= 1.0f;
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || 
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) direction.x += 1.0f;

        if (direction.x < -0.1f) {
            auto t = getComponent<TransformComponent>();
            if (t && t->getScale().x < 0.0f) {
                t->setScale({ 0.08f, 0.08f });
            }
        }
        else if (direction.x > 0.1f) {
            auto t = getComponent<TransformComponent>();
            if (t && t->getScale().x > 0.0f) {
                t->setScale({ -0.08f, 0.08f });
            }
        }

        if (auto movement = _movement.lock()) {
            movement->setVelocity(direction * _speed);
        }

        if ((sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) || 
             sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) && 
            _orbCooldownTimer <= 0.0f) {
            _orbCooldownTimer = _orbCooldown;
            shootOrb();
        }

        Actor::tick(deltaTime);
    }

    Possessable* getPossessedTarget() const { return _possessedTarget; }

private:
    void shootOrb() {
        auto t = getComponent<TransformComponent>();
        if (!t) return;

        sf::Vector2f startPos = t->getPosition();
        
        sf::Vector2i mousePos = sf::Mouse::getPosition(Window::getWindow());
        sf::Vector2f mouseWorldPos = Window::getWindow().mapPixelToCoords(mousePos);

        sf::Vector2f dir = mouseWorldPos - startPos;
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.0f) {
            dir /= len;
        } else {
            dir = { 1.0f, 0.0f };
        }

        // Offset spawn position slightly in the direction of fire to avoid spawning inside walls
        sf::Vector2f spawnPos = startPos + dir * 40.0f;

        World::getWorld().spawnActor<PossessionOrb>(spawnPos, dir, this);
        CameraManager::getInstance().shake(0.1f, 4.0f);
    }

    std::weak_ptr<MovementComponent> _movement;
    std::shared_ptr<CameraComponent> _camera;
    Possessable* _possessedTarget{ nullptr };
    
    float _speed{ 500.0f };
    float _orbCooldown{ 0.3f };
    float _orbCooldownTimer{ 0.0f };
};

// Circular dependency resolution inline method
inline void PossessionOrb::handlePossessTarget(Possessable* target) {
    _ownerPossessor->possess(target);
}
