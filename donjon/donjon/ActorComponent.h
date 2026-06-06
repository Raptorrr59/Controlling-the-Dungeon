#pragma once

class Actor;

class ActorComponent {
public:
    ActorComponent(Actor* owner) : _owner(owner) {}
    virtual ~ActorComponent() = default;

    Actor* getOwner() noexcept { return _owner; };

    virtual void beginPlay() {}
    virtual void tick(float deltaTime) {}

protected:
    Actor* _owner;
};