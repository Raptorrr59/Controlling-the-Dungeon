#pragma once

class Actor;

class ActorComponent {
public:
    ActorComponent(Actor* owner) : _owner(owner) {}
    virtual ~ActorComponent() = default;

    virtual void beginPlay() {}
    virtual void tick(float deltaTime) {}

protected:
    Actor* _owner;
};