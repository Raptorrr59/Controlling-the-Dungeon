#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include "ActorComponent.h"
#include "TransformComponent.h"
#include "RenderComponent.h"

#include <iostream>

class Actor : public sf::Drawable {
public:
    Actor(const sf::Vector2f& spawnPosition = { 0.0f, 0.0f }) {
        _transform = addComponent<TransformComponent>(spawnPosition);
    }

    virtual ~Actor() = default;

    Actor(const Actor&) = delete;
    Actor& operator=(const Actor&) = delete;

    virtual void beginPlay() {
        for (auto& pair : _components) {
            pair.second->beginPlay();
        }
    }

    virtual void tick(float deltaTime) {
        for (auto& pair : _components) {
            pair.second->tick(deltaTime);
        }
    }

    template<typename T, typename... Args>
    std::shared_ptr<T> addComponent(Args&&... args) {
        auto component = std::make_shared<T>(this, std::forward<Args>(args)...);
        _components[typeid(T)] = component;

        if (auto renderComp = std::dynamic_pointer_cast<RenderComponent>(component)) {
            _renderComponents.push_back(renderComp);
        }

        return component;
    }

    template<typename T>
    std::shared_ptr<T> getComponent() const {
        auto it = _components.find(typeid(T));
        if (it != _components.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        return nullptr;
    }

    template<typename T>
    std::vector<std::shared_ptr<T>> getAllComponents() const {
        std::vector<std::shared_ptr<T>> foundComponents;

        for (auto& pair : _components) {
            if (auto casted = std::dynamic_pointer_cast<T>(pair.second)) {
                foundComponents.push_back(casted);
            }
        }

        return foundComponents;
    }

    std::shared_ptr<TransformComponent> getTransform() const { return _transform; }

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        if (_transform) {
            states.transform.translate(_transform->getPosition());
            states.transform.scale(_transform->getScale());
        }

        for (const auto& renderComp : _renderComponents) {
            target.draw(*renderComp, states);
        }
    }

private:
    std::shared_ptr<TransformComponent> _transform;
    std::vector<std::shared_ptr<RenderComponent>> _renderComponents;
    std::unordered_map<std::type_index, std::shared_ptr<ActorComponent>> _components;
};