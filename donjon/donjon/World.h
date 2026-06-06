#pragma once
#include "Actor.h"
#include <vector>
#include <memory>

class World {
public:
    World(const World&) = delete;
    World& operator=(const World&) = delete;

    static World& getWorld() {
        static World world;
        return world;
    }

    template<typename T, typename... Args>
    std::weak_ptr<T> spawnActor(Args&&... args) {
        auto actor = std::make_shared<T>(std::forward<Args>(args)...);
        std::shared_ptr<T> actorPtr = actor;

        _actors.push_back(actor);
        actorPtr->beginPlay();

        return actorPtr;
    }

    void tick(float deltaTime) {
        for (auto& actor : _actors) {
            actor->tick(deltaTime);
        }
    }

    void draw(sf::RenderTarget& target) const {
        for (auto& actor : _actors) {
            target.draw(*actor);
        }
    }

private:
    World() = default;
    ~World() = default;

    std::vector<std::shared_ptr<Actor>> _actors;
};