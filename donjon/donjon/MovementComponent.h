#pragma once
#include "ActorComponent.h"
#include "Actor.h"
#include "BoxColliderComponent.h"
#include "TransformComponent.h"
#include "World.h"

class MovementComponent : public ActorComponent {
public:
    MovementComponent(Actor* owner) : ActorComponent(owner) {}

    void tick(float deltaTime) {
        auto actor = getOwner();
        auto transform = actor->getComponent<TransformComponent>();
        auto collider = actor->getComponent<BoxColliderComponent>();

        if (!transform) return;

        sf::Vector2f currentPos = transform->getPosition();
        currentPos += _velocity * deltaTime;
        transform->setPosition(currentPos);

        if (collider) {
            sf::Vector2f repulsion;
            if (World::getWorld().checkMoveCollision(*collider, repulsion)) {
                transform->setPosition(transform->getPosition() + repulsion);
            }
        }
    }

    void setVelocity(const sf::Vector2f& velocity) { _velocity = velocity; }

private:
    sf::Vector2f _velocity{ 0.0f, 0.0f };
};