#pragma once
#include "RenderComponent.h"
#include "SmartTextureLoader.h"
#include "AnimationData.h"
#include <string>
#include <unordered_map>

class SkeletalMeshComponent : public RenderComponent {
public:
    SkeletalMeshComponent(Actor* owner, const std::string& defaultTexturePath)
        : RenderComponent(owner),
        _sprite(SmartTextureLoader::getTexture(defaultTexturePath))
    {
    }

    void addAnimation(const std::string& name, const AnimationData& animData) {
        _animations[name] = animData;
        if (_currentAnimationName.empty()) {
            playAnimation(name);
        }
    }

    void playAnimation(const std::string& name) {
        if (_currentAnimationName == name) return;

        auto it = _animations.find(name);
        if (it != _animations.end()) {
            _currentAnimationName = name;
            _currentFrameIndex = 0;
            _elapsedTime = 0.0f;

            if (it->second.textureTarget) {
                _sprite.setTexture(*it->second.textureTarget);
            }

            _sprite.setTextureRect(it->second.frames[_currentFrameIndex]);
        }
    }

    void tick(float deltaTime) override {
        RenderComponent::tick(deltaTime);

        auto it = _animations.find(_currentAnimationName);
        if (it == _animations.end()) return;

        const AnimationData& anim = it->second;
        if (anim.frames.empty()) return;

        _elapsedTime += deltaTime;

        if (_elapsedTime >= anim.frameDuration) {
            _elapsedTime = 0.0f;
            _currentFrameIndex++;

            if (_currentFrameIndex >= anim.frames.size()) {
                if (anim.bLoops) {
                    _currentFrameIndex = 0;
                }
                else {
                    _currentFrameIndex = static_cast<int>(anim.frames.size()) - 1;
                }
            }

            _sprite.setTextureRect(anim.frames[_currentFrameIndex]);
        }
    }

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        target.draw(_sprite, states);
    }

private:
    sf::Sprite _sprite;
    std::unordered_map<std::string, AnimationData> _animations;
    std::string _currentAnimationName;
    int _currentFrameIndex{ 0 };
    float _elapsedTime{ 0.0f };
};