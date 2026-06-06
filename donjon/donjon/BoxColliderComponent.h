#pragma once
#include "RenderComponent.h" 
#include "TransformComponent.h"
#include "CollisionTypes.h"
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <memory>
#include <functional>

class BoxColliderComponent : public RenderComponent {
public:
    
    inline static bool bShowDebugColliders = true;

    BoxColliderComponent(Actor* owner, const sf::Vector2f& size = { 32.0f, 32.0f })
        : RenderComponent(owner), _size(size) {

        for (int i = 0; i < static_cast<int>(ECollisionChannel::MAX); ++i) {
            _responseMap[static_cast<ECollisionChannel>(i)] = ECollisionResponse::Block;
        }

        
        _debugShape.setSize(_size);
        _debugShape.setFillColor(sf::Color::Transparent);
        _debugShape.setOutlineThickness(1.0f);
        setDebugColorByChannel();
    }

    void beginPlay() override {
        _transform = getOwner()->getComponent<TransformComponent>();
    }

    void setCollisionObjectType(ECollisionChannel channel) {
        _objectChannel = channel;
        setDebugColorByChannel(); 
    }

    ECollisionChannel getCollisionObjectType() const { return _objectChannel; }

    void setCollisionResponse(ECollisionChannel channel, ECollisionResponse response) {
        _responseMap[channel] = response;
    }

    ECollisionResponse getResponseToChannel(ECollisionChannel channel) const {
        auto it = _responseMap.find(channel);
        return (it != _responseMap.end()) ? it->second : ECollisionResponse::Block;
    }

    sf::FloatRect getGlobalBounds() const {
        sf::Vector2f pos{ 0.0f, 0.0f };
        sf::Vector2f scale{ 1.0f, 1.0f };

        if (auto t = _transform.lock()) {
            pos = t->getPosition();
            scale = t->getScale();
        }

        
        sf::Vector2f scaledSize(_size.x * scale.x, _size.y * scale.y);

        
        return sf::FloatRect(pos, scaledSize);
    }

    std::function<void(Actor* otherActor)> onComponentBeginOverlap;

protected:
    
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        if (!bShowDebugColliders) return;

        if (auto t = _transform.lock()) {
            
            sf::Vector2f scale = t->getScale();

            float thicknessX = (scale.x != 0.0f) ? (1.0f / scale.x) : 1.0f;
            _debugShape.setOutlineThickness(thicknessX);

            target.draw(_debugShape, states);
        }
    }
private:
    void setDebugColorByChannel() {
        switch (_objectChannel) {
        case ECollisionChannel::WorldStatic:
            _debugShape.setOutlineColor(sf::Color::Red); 
            break;
        case ECollisionChannel::Pawn:
            _debugShape.setOutlineColor(sf::Color::Green); 
            break;
        case ECollisionChannel::Projectile:
            _debugShape.setOutlineColor(sf::Color::Magenta); 
            break;
        case ECollisionChannel::Trigger:
            _debugShape.setOutlineColor(sf::Color::Yellow); 
            break;
        default:
            _debugShape.setOutlineColor(sf::Color::White);
            break;
        }
    }

    sf::Vector2f _size;
    std::weak_ptr<TransformComponent> _transform;
    ECollisionChannel _objectChannel{ ECollisionChannel::WorldStatic };
    std::unordered_map<ECollisionChannel, ECollisionResponse> _responseMap;

    mutable sf::RectangleShape _debugShape;
};