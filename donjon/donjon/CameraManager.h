#pragma once
#include "CameraComponent.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <cmath>
#include <iostream>

class CameraManager {
public:
    CameraManager(const CameraManager&) = delete;
    CameraManager& operator=(const CameraManager&) = delete;

    static CameraManager& getInstance() {
        static CameraManager instance;
        return instance;
    }

    void setActiveCamera(std::shared_ptr<CameraComponent> camera) {
        _activeCamera = camera;
    }

    std::shared_ptr<CameraComponent> getActiveCamera() const {
        return _activeCamera.lock();
    }

    void shake(float duration, float intensity) {
        _shakeDuration = duration;
        _shakeIntensity = intensity;
        _shakeTimer = duration;
    }

    void tick(float deltaTime) {
        std::cout << _shakeTimer << std::endl;;
        auto cam = _activeCamera.lock();
        if (!cam) return;

        if (_shakeTimer > 0.0f) {
            _shakeTimer -= deltaTime;

            if (_shakeTimer < 0.0f) {
                _shakeTimer = 0.0f;
            }
        }
    }

    void applyView(sf::RenderWindow& window) {
        if (auto cam = _activeCamera.lock()) {
            sf::View finalView = cam->getView();

            if (_shakeTimer > 0.0f && _shakeDuration > 0.0f) {
                float currentIntensity = (_shakeTimer / _shakeDuration) * _shakeIntensity;

                float offsetX = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f) * currentIntensity;
                float offsetY = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f) * currentIntensity;

                finalView.move({ offsetX, offsetY });
            }

            window.setView(finalView);
        }
        else {
            window.setView(window.getDefaultView());
        }
    }

private:
    CameraManager() = default;
    ~CameraManager() = default;

    std::weak_ptr<CameraComponent> _activeCamera;
    sf::View _currentView;

    float _shakeDuration{ 0.0f };
    float _shakeIntensity{ 0.0f };
    float _shakeTimer{ 0.0f };
};