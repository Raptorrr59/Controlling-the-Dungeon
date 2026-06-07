#pragma once
#include "ActorComponent.h"
#include "Actor.h"
#include "BoxColliderComponent.h"
#include "TransformComponent.h"
#include "World.h"
#include <cmath>

class PlatformerMovementComponent : public ActorComponent {
public:
    PlatformerMovementComponent(Actor* owner) : ActorComponent(owner) {}

    void tick(float deltaTime) override {
        auto actor = getOwner();
        auto transform = actor->getComponent<TransformComponent>();
        auto collider = actor->getComponent<BoxColliderComponent>();

        if (!transform) return;

        // Apply gravity if enabled
        if (_bUseGravity) {
            _velocity.y += _gravity * deltaTime;
        }

        // Limit terminal velocity
        if (_velocity.y > 1800.0f) {
            _velocity.y = 1800.0f;
        }

        // Horizontal movement & collision
        float dx = _velocity.x * deltaTime;
        if (std::abs(dx) > 0.0001f) {
            transform->setPosition(transform->getPosition() + sf::Vector2f(dx, 0.0f));
            if (collider) {
                sf::Vector2f repulsion;
                if (World::getWorld().checkMoveCollision(*collider, repulsion)) {
                    transform->setPosition(transform->getPosition() + sf::Vector2f(repulsion.x, 0.0f));
                    _velocity.x = 0.0f;
                }
            }
        }

        // Vertical movement & collision
        float dy = _velocity.y * deltaTime;
        _bIsOnGround = false;
        if (std::abs(dy) > 0.0001f) {
            transform->setPosition(transform->getPosition() + sf::Vector2f(0.0f, dy));
            if (collider) {
                sf::Vector2f repulsion;
                if (World::getWorld().checkMoveCollision(*collider, repulsion)) {
                    transform->setPosition(transform->getPosition() + sf::Vector2f(0.0f, repulsion.y));
                    if (repulsion.y < 0.0f) { // Pushed up -> landed on ground
                        _bIsOnGround = true;
                        _velocity.y = 0.0f;
                    }
                    else if (repulsion.y > 0.0f) { // Pushed down -> hit ceiling
                        _velocity.y = 0.0f;
                    }
                }
            }
        }
    }

    void setVelocity(const sf::Vector2f& velocity) { _velocity = velocity; }
    const sf::Vector2f& getVelocity() const { return _velocity; }

    void addVelocity(const sf::Vector2f& amount) { _velocity += amount; }

    void setGravityEnabled(bool enabled) { _bUseGravity = enabled; }
    bool isGravityEnabled() const { return _bUseGravity; }

    void setGravity(float gravity) { _gravity = gravity; }
    float getGravity() const { return _gravity; }

    bool isOnGround() const { return _bIsOnGround; }

private:
    sf::Vector2f _velocity{ 0.0f, 0.0f };
    float _gravity{ 2200.0f };
    bool _bUseGravity{ true };
    bool _bIsOnGround{ false };
};
