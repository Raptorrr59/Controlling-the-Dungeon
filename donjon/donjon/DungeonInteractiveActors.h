#pragma once
#include "Actor.h"
#include "BoxColliderComponent.h"
#include "ShapeComponent.h"
#include "World.h"
#include "Possessor.h"
#include "Possessable.h"
#include "WizardProjectile.h"
#include <vector>
#include <functional>

// Forward declarations
class Gate;

// ==========================================
// 1. GATE ACTOR
// ==========================================
class Gate : public Actor {
public:
    Gate(const sf::Vector2f& spawnPosition, const sf::Vector2f& size = { 40.0f, 200.0f })
        : Actor(spawnPosition), _size(size) {
        
        _shape = addComponent<ShapeComponent>();
        _shape->setShape(_size, sf::Color(100, 100, 120)); // Iron gray

        _col = addComponent<BoxColliderComponent>(_size);
        _col->setCollisionObjectType(ECollisionChannel::WorldStatic);
        _col->setCollisionResponse(ECollisionChannel::Pawn, ECollisionResponse::Block);
        _col->setCollisionResponse(ECollisionChannel::Projectile, ECollisionResponse::Block);
    }

    void setOpen(bool open) {
        if (_bIsOpen == open) return;
        _bIsOpen = open;

        if (_bIsOpen) {
            // Disable blocking
            _col->setCollisionResponse(ECollisionChannel::Pawn, ECollisionResponse::Ignore);
            _col->setCollisionResponse(ECollisionChannel::Projectile, ECollisionResponse::Ignore);
            _shape->setShape(_size, sf::Color(100, 100, 120, 50)); // Semi-transparent
        } else {
            // Enable blocking
            _col->setCollisionResponse(ECollisionChannel::Pawn, ECollisionResponse::Block);
            _col->setCollisionResponse(ECollisionChannel::Projectile, ECollisionResponse::Block);
            _shape->setShape(_size, sf::Color(100, 100, 120, 255)); // Solid
        }
        CameraManager::getInstance().shake(0.1f, 3.0f);
    }

    bool isOpen() const { return _bIsOpen; }

private:
    sf::Vector2f _size;
    std::shared_ptr<ShapeComponent> _shape;
    std::shared_ptr<BoxColliderComponent> _col;
    bool _bIsOpen{ false };
};

// ==========================================
// 2. PRESSURE PLATE
// ==========================================
class PressurePlate : public Actor {
public:
    PressurePlate(const sf::Vector2f& spawnPosition, Gate* targetGate, const sf::Vector2f& size = { 100.0f, 20.0f })
        : Actor(spawnPosition), _targetGate(targetGate), _size(size) {
        
        _shape = addComponent<ShapeComponent>();
        _shape->setShape(_size, sf::Color::Red);

        auto col = addComponent<BoxColliderComponent>(_size);
        col->setCollisionObjectType(ECollisionChannel::Trigger);
        col->setCollisionResponse(ECollisionChannel::Pawn, ECollisionResponse::Overlap);

        col->onComponentBeginOverlap = [this](Actor* other) {
            if (other) {
                // If it is a pawn (either Ghost or Possessable), trigger it
                _bIsPressedThisFrame = true;
            }
        };
    }

    void tick(float deltaTime) override {
        if (_bIsPressedThisFrame) {
            if (!_bIsPressed) {
                _bIsPressed = true;
                if (_targetGate) _targetGate->setOpen(true);
                _shape->setShape(_size, sf::Color::Green);
            }
            _bIsPressedThisFrame = false; // Reset for next overlap check
        } else {
            if (_bIsPressed) {
                _bIsPressed = false;
                if (_targetGate) _targetGate->setOpen(false);
                _shape->setShape(_size, sf::Color::Red);
            }
        }

        Actor::tick(deltaTime);
    }

private:
    Gate* _targetGate;
    sf::Vector2f _size;
    std::shared_ptr<ShapeComponent> _shape;
    bool _bIsPressedThisFrame{ false };
    bool _bIsPressed{ false };
};

// ==========================================
// 3. TRIGGER BUTTON
// ==========================================
class TriggerButton : public Actor {
public:
    TriggerButton(const sf::Vector2f& spawnPosition, Gate* targetGate, const sf::Vector2f& size = { 40.0f, 40.0f })
        : Actor(spawnPosition), _targetGate(targetGate), _size(size) {
        
        _shape = addComponent<ShapeComponent>();
        _shape->setShape(_size, sf::Color(200, 100, 0)); // Orange

        auto col = addComponent<BoxColliderComponent>(_size);
        col->setCollisionObjectType(ECollisionChannel::Trigger);
        col->setCollisionResponse(ECollisionChannel::Projectile, ECollisionResponse::Overlap);

        col->onComponentBeginOverlap = [this](Actor* other) {
            if (other && !other->isPendingKill()) {
                if (auto proj = dynamic_cast<WizardProjectile*>(other)) {
                    toggleGate();
                    proj->destroy(); // Destroy fireball immediately to avoid double toggle
                }
            }
        };
    }

private:
    void toggleGate() {
        if (_targetGate) {
            _targetGate->setOpen(!_targetGate->isOpen());
            if (_targetGate->isOpen()) {
                _shape->setShape(_size, sf::Color::Cyan);
            } else {
                _shape->setShape(_size, sf::Color(200, 100, 0));
            }
        }
    }

    Gate* _targetGate;
    sf::Vector2f _size;
    std::shared_ptr<ShapeComponent> _shape;
};

// ==========================================
// 4. SPIKES HAZARD
// ==========================================
class Spikes : public Actor {
public:
    Spikes(const sf::Vector2f& spawnPosition, const sf::Vector2f& size = { 120.0f, 30.0f }, std::function<void()> onTriggerCallback = nullptr)
        : Actor(spawnPosition), _size(size), _onTrigger(onTriggerCallback) {
        
        _shape = addComponent<ShapeComponent>();
        _shape->setShape(_size, sf::Color(180, 50, 50)); // Red hazard

        auto col = addComponent<BoxColliderComponent>(_size);
        col->setCollisionObjectType(ECollisionChannel::Trigger);
        col->setCollisionResponse(ECollisionChannel::Pawn, ECollisionResponse::Overlap);

        col->onComponentBeginOverlap = [this](Actor* other) {
            if (!other) return;

            // Only hurt Ghost or Wizard. Knight is immune!
            bool bIsPossessor = (dynamic_cast<Possessor*>(other) != nullptr);
            bool bIsWizard = (dynamic_cast<PossessableWizard*>(other) != nullptr);
            
            // If it's a possessor, check if they are in ghost form
            if (bIsPossessor) {
                auto possessor = dynamic_cast<Possessor*>(other);
                if (possessor->getPossessedTarget() == nullptr) {
                    // Ghost form touched spikes!
                    if (_onTrigger) _onTrigger();
                }
            } else if (bIsWizard) {
                // Wizard touched spikes!
                if (_onTrigger) _onTrigger();
            }
        };
    }

private:
    sf::Vector2f _size;
    std::shared_ptr<ShapeComponent> _shape;
    std::function<void()> _onTrigger;
};

// ==========================================
// 5. GHOST BARRIER
// ==========================================
class GhostBarrier : public Actor {
public:
    GhostBarrier(const sf::Vector2f& spawnPosition, const sf::Vector2f& size = { 30.0f, 150.0f })
        : Actor(spawnPosition), _size(size) {
        
        _shape = addComponent<ShapeComponent>();
        _shape->setShape(_size, sf::Color(0, 191, 255, 128)); // Glowing transparent neon blue

        auto col = addComponent<BoxColliderComponent>(_size);
        col->setCollisionObjectType(ECollisionChannel::Trigger);
        col->setCollisionResponse(ECollisionChannel::Pawn, ECollisionResponse::Overlap);

        col->onComponentBeginOverlap = [this](Actor* other) {
            if (!other) return;

            if (auto possessor = dynamic_cast<Possessor*>(other)) {
                // If the player is in ghost form (no possessed body), zap them back!
                if (possessor->getPossessedTarget() == nullptr) {
                    auto t = possessor->getTransform();
                    if (t) {
                        // Push back the ghost!
                        sf::Vector2f pushDir = t->getPosition() - getTransform()->getPosition();
                        float len = std::sqrt(pushDir.x * pushDir.x + pushDir.y * pushDir.y);
                        if (len > 0.0f) {
                            pushDir /= len;
                            t->setPosition(t->getPosition() + pushDir * 80.0f);
                        } else {
                            t->setPosition(t->getPosition() + sf::Vector2f(-80.0f, 0.0f));
                        }
                        CameraManager::getInstance().shake(0.2f, 8.0f);
                    }
                }
            }
        };
    }

private:
    sf::Vector2f _size;
    std::shared_ptr<ShapeComponent> _shape;
};

// ==========================================
// 6. PORTAL (EXIT)
// ==========================================
class Portal : public Actor {
public:
    Portal(const sf::Vector2f& spawnPosition, std::function<void()> onWinCallback = nullptr, const sf::Vector2f& size = { 80.0f, 120.0f })
        : Actor(spawnPosition), _size(size), _onWin(onWinCallback) {
        
        _shape = addComponent<ShapeComponent>();
        _shape->setShape(_size, sf::Color(0, 255, 150, 180)); // Cyan-green glowing portal

        auto col = addComponent<BoxColliderComponent>(_size);
        col->setCollisionObjectType(ECollisionChannel::Trigger);
        col->setCollisionResponse(ECollisionChannel::Pawn, ECollisionResponse::Overlap);

        col->onComponentBeginOverlap = [this](Actor* other) {
            if (!other) return;

            if (auto possessor = dynamic_cast<Possessor*>(other)) {
                // Must be in ghost form to win (cannot enter with a body)
                if (possessor->getPossessedTarget() == nullptr) {
                    if (_onWin) _onWin();
                }
            }
        };
    }

private:
    sf::Vector2f _size;
    std::shared_ptr<ShapeComponent> _shape;
    std::function<void()> _onWin;
};
