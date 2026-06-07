#pragma once
#include "Possessable.h"
#include "PlatformerMovementComponent.h"
#include "SkeletalMeshComponent.h"
#include "CameraManager.h"
#include "WizardProjectile.h"
#include "Window.h"
#include "PossessableKnight.h"
#include <cmath>

class PossessableWizard : public Possessable {
public:
    PossessableWizard(const sf::Vector2f& spawnPosition = { 0.0f, 0.0f }) : Possessable(spawnPosition) {
        _movement = addComponent<PlatformerMovementComponent>();
        
        _camera = addComponent<CameraComponent>(sf::Vector2f(1920.f, 1080.f));
        _camera->setZoom(1.2f); // Zoom out camera view

        // Raw wizard dimensions (55 width, 83 height) scaled by 1.2f, centered
        auto col = addComponent<BoxColliderComponent>(sf::Vector2f(55.f, 83.f), true);
        col->setCollisionObjectType(ECollisionChannel::Pawn);

        col->setCollisionResponse(ECollisionChannel::WorldStatic, ECollisionResponse::Block);
        col->setCollisionResponse(ECollisionChannel::WorldDynamic, ECollisionResponse::Block);
        col->setCollisionResponse(ECollisionChannel::Pawn, ECollisionResponse::Ignore);
        col->setCollisionResponse(ECollisionChannel::Trigger, ECollisionResponse::Overlap);

        auto animMesh = addComponent<SkeletalMeshComponent>("../assets/wizard_idle.png");
        animMesh->addAnimation("Idle", AnimationData::detectFlipbook("../assets/wizard_idle.png", 1.6f, true));
        animMesh->addAnimation("Walk", AnimationData::detectFlipbook("../assets/wizard_run.png", 0.8f, true));
        
        auto t = this->getComponent<TransformComponent>();
        t->setScale({ 1.2f, 1.2f }); 

        _skeletalMesh = animMesh;
    }

    void onPossess() override {
        Possessable::onPossess();
        CameraManager::getInstance().setActiveCamera(_camera);
    }

    void tick(float deltaTime) override {
        if (_fireCooldownTimer > 0.0f) {
            _fireCooldownTimer -= deltaTime;
        }

        auto movement = _movement.lock();
        if (!movement) {
            Possessable::tick(deltaTime);
            return;
        }

        // Detect if we can ride any Knight
        bool foundKnight = false;
        sf::Vector2f knightVel{ 0.0f, 0.0f };
        sf::Vector2f knightPos{ 0.0f, 0.0f };
        float kHalfHeight = 0.0f;
        float wHalfHeight = 0.0f;

        auto knights = World::getWorld().getActorsOfClass<PossessableKnight>();
        auto wizardCol = getComponent<BoxColliderComponent>();

        for (auto knight : knights) {
            if (knight) {
                auto knightCol = knight->getComponent<BoxColliderComponent>();
                if (wizardCol && knightCol) {
                    sf::FloatRect wRect = wizardCol->getGlobalBounds();
                    sf::FloatRect kRect = knightCol->getGlobalBounds();

                    // Check horizontal overlap
                    bool horizOverlap = (wRect.position.x < kRect.position.x + kRect.size.x) &&
                                        (wRect.position.x + wRect.size.x > kRect.position.x);

                    // Check vertical resting: bottom of wizard is near top of knight
                    float wizardBottom = wRect.position.y + wRect.size.y;
                    float knightTop = kRect.position.y;
                    float diff = wizardBottom - knightTop;
                    bool isClose = (diff >= -10.0f && diff <= 15.0f);

                    sf::Vector2f kVel{ 0.0f, 0.0f };
                    if (auto knightMovement = knight->getComponent<PlatformerMovementComponent>()) {
                        kVel = knightMovement->getVelocity();
                    }

                    if (horizOverlap && (isClose || _bIsRidingKnight)) {
                        // Check if we are falling down onto the knight, or already riding
                        if (_bIsRidingKnight || movement->getVelocity().y >= kVel.y) {
                            foundKnight = true;
                            knightVel = kVel;
                            if (auto knightTransform = knight->getTransform()) {
                                knightPos = knightTransform->getPosition();
                            }
                            kHalfHeight = kRect.size.y / 2.0f;
                            wHalfHeight = wRect.size.y / 2.0f;
                            break;
                        }
                    }
                }
            }
        }

        bool bHoldingJump = false;
        if (_bIsPossessed) {
            bHoldingJump = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) || 
                           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z) || 
                           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || 
                           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);
        }

        // Apply horizontal/vertical/jump velocity
        sf::Vector2f velocity = movement->getVelocity();
        velocity.x = 0.0f;

        bool hasHorizontalInput = false;
        if (_bIsPossessed) {
            hasHorizontalInput = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q) || 
                                 sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || 
                                 sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || 
                                 sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || 
                                 sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);
        }

        if (foundKnight) {
            _bIsRidingKnight = true;
            movement->setGravityEnabled(false);

            auto t = getTransform();
            if (t) {
                if (hasHorizontalInput) {
                    // Only snap Y position to stay on head, allow horizontal walking
                    t->setPosition({ t->getPosition().x, knightPos.y - kHalfHeight - wHalfHeight });
                } else {
                    // Snap both X and Y to Knight center
                    t->setPosition({ knightPos.x, knightPos.y - kHalfHeight - wHalfHeight });
                }
            }

            // Set base velocity to match Knight
            velocity.y = knightVel.y;
            if (!hasHorizontalInput) {
                velocity.x = knightVel.x;
            }
        } else {
            _bIsRidingKnight = false;
            movement->setGravityEnabled(true);
        }

        // Glide feature
        if (_bIsPossessed && bHoldingJump && velocity.y > 0.0f) {
            movement->setGravity(_glideGravity);
        } else {
            movement->setGravity(_normalGravity);
        }

        if (_bIsPossessed) {
            // Horizontal control
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q) || 
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || 
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
                velocity.x = -_speed;
                auto t = getComponent<TransformComponent>();
                if (t && t->getScale().x > 0.0f) {
                    t->setScale({ -1.2f, 1.2f });
                }
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || 
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
                velocity.x = _speed;
                auto t = getComponent<TransformComponent>();
                if (t && t->getScale().x < 0.0f) {
                    t->setScale({ 1.2f, 1.2f });
                }
            }

            // Jump control (allow jump if on ground or riding knight)
            if (bHoldingJump && (movement->isOnGround() || _bIsRidingKnight)) {
                velocity.y = -_jumpForce;
                _bIsRidingKnight = false;
                movement->setGravityEnabled(true);
            }

            // Shoot control (Mouse Left Click or E)
            if ((sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) && _fireCooldownTimer <= 0.0f) {
                _fireCooldownTimer = _fireCooldown;
                shootProjectile();
            }
        }

        movement->setVelocity(velocity);

        if (auto mesh = _skeletalMesh.lock()) {
            if (std::abs(velocity.x) > 1.0f) {
                mesh->playAnimation("Walk");
            } else {
                mesh->playAnimation("Idle");
            }
        }

        Possessable::tick(deltaTime);
    }

private:
    void shootProjectile() {
        auto t = getComponent<TransformComponent>();
        if (!t) return;

        sf::Vector2f wizardCenter = t->getPosition() + sf::Vector2f(0.0f, -40.0f); // offset upwards to align with hands
        
        sf::Vector2i mousePos = sf::Mouse::getPosition(Window::getWindow());
        sf::Vector2f mouseWorldPos = Window::getWindow().mapPixelToCoords(mousePos);

        sf::Vector2f dir = mouseWorldPos - wizardCenter;
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.0f) {
            dir /= len;
        } else {
            dir = { 1.0f, 0.0f };
        }

        // Spawn fireball projectile
        World::getWorld().spawnActor<WizardProjectile>(wizardCenter, dir);
        
        // Add a slight screenshake for premium feel!
        CameraManager::getInstance().shake(0.15f, 6.0f);
    }

    std::weak_ptr<PlatformerMovementComponent> _movement;
    std::weak_ptr<SkeletalMeshComponent> _skeletalMesh;
    std::shared_ptr<CameraComponent> _camera;
    float _speed{ 300.0f };      // Wizard is slower than Knight
    float _jumpForce{ 750.0f };  // Wizard jumps slightly lower than Knight but can glide
    float _normalGravity{ 2200.0f };
    float _glideGravity{ 350.0f }; // Glides down slowly when jump is held

    float _fireCooldown{ 0.4f };
    float _fireCooldownTimer{ 0.0f };
    bool _bIsRidingKnight{ false };
};
