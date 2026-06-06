#pragma once
#include "ActorComponent.h"
#include "Actor.h"
#include "TransformComponent.h"

class MovementComponent : public ActorComponent {
public:
    MovementComponent(Actor* owner) : ActorComponent(owner) {}

    void tick(float deltaTime) override {
        if (auto transform = _owner->getTransform()) {
            transform->setPosition(transform->getPosition() + _velocity * deltaTime);
        }
    }

    void setVelocity(const sf::Vector2f& velocity) { _velocity = velocity; }

private:
    sf::Vector2f _velocity{ 0.0f, 0.0f };
};