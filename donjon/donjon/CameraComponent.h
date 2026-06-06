#pragma once
#include "ActorComponent.h"
#include "TransformComponent.h"
#include <SFML/Graphics.hpp>
#include <memory>

class CameraComponent : public ActorComponent {
public:
    CameraComponent(Actor* owner, const sf::Vector2f& windowSize)
        : ActorComponent(owner),
        _baseSize(windowSize),
        _view(sf::FloatRect({ 0.0f, 0.0f }, windowSize)) {
    }

    void beginPlay() override {
        _transform = getOwner()->getComponent<TransformComponent>();
    }

    void tick(float deltaTime) override {
        if (auto t = _transform.lock()) {
            _view.setCenter(t->getPosition());
        }
    }

    void setZoom(float factor) {
        _currentZoom = factor;
        _view.setSize({ _baseSize.x * _currentZoom, _baseSize.y * _currentZoom });
    }

    const sf::View& getView() const { return _view; }
    sf::View& getModifiableView() { return _view; }

private:
    sf::View _view;
    sf::Vector2f _baseSize;
    float _currentZoom{ 1.0f };
    std::weak_ptr<TransformComponent> _transform;
};