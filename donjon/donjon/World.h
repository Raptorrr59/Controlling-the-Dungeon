#pragma once
#include "Actor.h"
#include "BoxColliderComponent.h"
#include <vector>
#include <memory>
#include <ranges>
#include <concepts>
#include <algorithm>
#include "CameraManager.h"

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
        _actors.push_back(actor);
        actor->beginPlay();
        return actor;
    }

    void tick(float deltaTime) {
        for (auto& actor : _actors) {
            actor->tick(deltaTime);
        }

        CameraManager::getInstance().tick(deltaTime);
        processOverlaps();
    }

    void draw(sf::RenderTarget& target) const {
        CameraManager::getInstance().applyView(Window::getWindow());
        for (auto& actor : _actors) {
            target.draw(*actor);
        }
    }

    auto getAllActors() {
        return _actors | std::views::all;
    }

    template<typename T>
    auto getActorsOfClass() {
        return _actors
            | std::views::filter([](const auto& actor) {
            return dynamic_cast<T*>(actor.get()) != nullptr;
                })
            | std::views::transform([](const auto& actor) {
            return std::static_pointer_cast<T>(actor);
                });
    }

    template<typename T>
    auto getActorsWithComponent() {
        return _actors | std::views::filter([](const auto& actor) {
            return actor->template getComponent<T>() != nullptr;
            });
    }

    template<typename... Components>
        requires (sizeof...(Components) > 0)
    auto getActorsWithComponents() {
        return _actors | std::views::filter([](const auto& actor) {
            return ((actor->template getComponent<Components>() != nullptr) && ...);
            });
    }


    bool checkMoveCollision(const BoxColliderComponent& targetCollider, sf::Vector2f& outRepulsion) {
        outRepulsion = { 0.0f, 0.0f };

        auto actorRange = getActorsWithComponent<BoxColliderComponent>();

        std::vector<std::shared_ptr<Actor>> targetActors;
        for (auto actor : actorRange) {
            targetActors.push_back(actor);
        }

        sf::FloatRect targetRect = targetCollider.getGlobalBounds();

        for (auto& actor : targetActors) {
            auto other = actor->getComponent<BoxColliderComponent>();
            if (!other || other.get() == &targetCollider) continue;

            if (targetCollider.getResponseToChannel(other->getCollisionObjectType()) != ECollisionResponse::Block ||
                other->getResponseToChannel(targetCollider.getCollisionObjectType()) != ECollisionResponse::Block) {
                continue;
            }

            sf::FloatRect otherRect = other->getGlobalBounds();
            if (auto intersection = targetRect.findIntersection(otherRect)) {
                sf::Vector2f targetCenter = targetRect.position + (targetRect.size / 2.0f);
                sf::Vector2f otherCenter = otherRect.position + (otherRect.size / 2.0f);

                if (intersection->size.x < intersection->size.y) {
                    outRepulsion.x = (targetCenter.x > otherCenter.x) ? intersection->size.x : -intersection->size.x;
                }
                else {
                    outRepulsion.y = (targetCenter.y > otherCenter.y) ? intersection->size.y : -intersection->size.y;
                }
                return true;
            }
        }
        return false;
    }

    void processOverlaps() {
        auto actorRange = getActorsWithComponent<BoxColliderComponent>();

        std::vector<std::shared_ptr<Actor>> targetActors;
        for (auto actor : actorRange) {
            targetActors.push_back(actor);
        }

        if (targetActors.size() < 2) return;

        for (size_t i = 0; i < targetActors.size(); ++i) {
            auto colA = targetActors[i]->getComponent<BoxColliderComponent>();
            if (!colA) continue;

            for (size_t j = i + 1; j < targetActors.size(); ++j) {
                auto colB = targetActors[j]->getComponent<BoxColliderComponent>();
                if (!colB) continue;

                bool wantsOverlapA = colA->getResponseToChannel(colB->getCollisionObjectType()) == ECollisionResponse::Overlap;
                bool wantsOverlapB = colB->getResponseToChannel(colA->getCollisionObjectType()) == ECollisionResponse::Overlap;
                if ((wantsOverlapA || wantsOverlapB) && colA->getGlobalBounds().findIntersection(colB->getGlobalBounds()).has_value()) {
                    if (wantsOverlapA && colA->onComponentBeginOverlap) {
                        colA->onComponentBeginOverlap(colB->getOwner());
                    }
                    if (wantsOverlapB && colB->onComponentBeginOverlap) {
                        colB->onComponentBeginOverlap(colA->getOwner());
                    }
                }
            }
        }
    }

private:
    World() = default;
    ~World() = default;

    std::vector<std::shared_ptr<Actor>> _actors;
};