#pragma once
#include "Actor.h"

class Possessable : public Actor {
public:
    Possessable(const sf::Vector2f& spawnPosition = { 0.0f, 0.0f }) : Actor(spawnPosition) {}
    virtual ~Possessable() = default;

    virtual void onPossess() { 
        _bIsPossessed = true; 
    }
    
    virtual void onUnpossess() { 
        _bIsPossessed = false; 
    }
    
    bool isPossessed() const { return _bIsPossessed; }

protected:
    bool _bIsPossessed{ false };
};
