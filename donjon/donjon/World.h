#pragma once
#include "Actor.h"
#include <vector>
#include <memory>
#include <ranges>
#include <concepts>

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
	}

	void draw(sf::RenderTarget& target) const {
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
			return !actor->getComponent<T>().expired();
			});
	}

	template<typename... Components>
		requires (sizeof...(Components) > 0)
	auto getActorsWithComponents() {
		return _actors | std::views::filter([](const auto& actor) {
			return (!actor->getComponent<Components>().expired() && ...);
			});
	}

private:
	World() = default;
	~World() = default;

	std::vector<std::shared_ptr<Actor>> _actors;
};