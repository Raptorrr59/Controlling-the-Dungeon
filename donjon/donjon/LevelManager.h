#pragma once
#include "World.h"
#include "SolidBlock.h"
#include "Possessor.h"
#include "PossessableKnight.h"
#include "PossessableWizard.h"
#include "DungeonInteractiveActors.h"
#include "StoryScreen.h"
#include "EndScreen.h"
#include <vector>
#include <string>
#include <iostream>
#include <memory>

class LevelManager {
public:
    static LevelManager& getInstance() {
        static LevelManager instance;
        return instance;
    }

    void loadLevel(int levelIndex) {
        _currentLevelIndex = levelIndex;
        World::getWorld().clearAllActors();

        std::cout << "Loading Level " << levelIndex << std::endl;

        switch (levelIndex) {
            case 0:
                buildIntroScreen();
                break;
            case 1:
                buildLevel1();
                break;
            case 2:
                buildLevel2();
                break;
            case 3:
                buildLevel3();
                break;
            default:
                buildWinScreen();
                break;
        }
    }

    void queueLevelLoad(int levelIndex) {
        _pendingLevelLoad = levelIndex;
    }

    void processPendingLevelLoad() {
        if (_pendingLevelLoad >= 0) {
            int targetLevel = _pendingLevelLoad;
            _pendingLevelLoad = -1;
            loadLevel(targetLevel);
        }
    }

    void restartCurrentLevel() {
        queueLevelLoad(_currentLevelIndex);
    }

    void nextLevel() {
        queueLevelLoad(_currentLevelIndex + 1);
    }

    int getCurrentLevelIndex() const { return _currentLevelIndex; }

private:
    LevelManager() = default;

    void spawnBorderWalls(int widthInCells, int heightInCells, const sf::Color& color = sf::Color(35, 40, 55)) {
        float size = 80.0f;
        // Floor
        World::getWorld().spawnActor<SolidBlock>(sf::Vector2f(0.0f, heightInCells * size), sf::Vector2f(widthInCells * size, size), color);
        // Ceiling
        World::getWorld().spawnActor<SolidBlock>(sf::Vector2f(0.0f, -size), sf::Vector2f(widthInCells * size, size), color);
        // Left Wall
        World::getWorld().spawnActor<SolidBlock>(sf::Vector2f(-size, 0.0f), sf::Vector2f(size, heightInCells * size), color);
        // Right Wall
        World::getWorld().spawnActor<SolidBlock>(sf::Vector2f(widthInCells * size, 0.0f), sf::Vector2f(size, heightInCells * size), color);
    }

    void buildLevel1() {
        float size = 80.0f;
        spawnBorderWalls(25, 12);

        // MIDDLE WALL
        // x = 10, y = 5 to 11
        World::getWorld().spawnActor<SolidBlock>(sf::Vector2f(10 * size, 5 * size), sf::Vector2f(size, 7 * size));

        // Spawn Gate
        auto gate = World::getWorld().spawnActor<Gate>(sf::Vector2f(17 * size, 6 * size), sf::Vector2f(size, 6 * size)).lock();

        // Spawn Pressure Plate (linked to gate)
        World::getWorld().spawnActor<PressurePlate>(sf::Vector2f(13 * size, 11.75f * size), gate.get(), sf::Vector2f(1.5f * size, 20.0f));

        // Spawn Knight
        World::getWorld().spawnActor<PossessableKnight>(sf::Vector2f(11.5f * size, 10.0f * size));

        // Spawn Portal
        World::getWorld().spawnActor<Portal>(sf::Vector2f(21 * size, 10.0f * size), [this]() {
            nextLevel();
        });

        // Spawn Spikes (to teach danger, though not strictly required here)
        World::getWorld().spawnActor<Spikes>(sf::Vector2f(6 * size, 11.6f * size), sf::Vector2f(2 * size, 32.0f), [this]() {
            restartCurrentLevel();
        });

        // Spawn Player Ghost
        auto ghost = World::getWorld().spawnActor<Possessor>(sf::Vector2f(2 * size, 8 * size));
    }

    void buildLevel2() {
        float size = 80.0f;
        spawnBorderWalls(35, 12);

        // Spikes in the middle (x = 12 to 19)
        World::getWorld().spawnActor<Spikes>(sf::Vector2f(12 * size, 11.6f * size), sf::Vector2f(7 * size, 32.0f), [this]() {
            restartCurrentLevel();
        });

        // Anti-Ghost Laser Barrier at x = 10 (blocking from y = 0 to 12)
        auto barrier = World::getWorld().spawnActor<GhostBarrier>(sf::Vector2f(10 * size, 0.0f), sf::Vector2f(30.0f, 12 * size)).lock();

        // Gate at x = 21 (blocking the Knight)
        auto gate = World::getWorld().spawnActor<Gate>(sf::Vector2f(21 * size, 6 * size), sf::Vector2f(size, 6 * size)).lock();

        // Trigger Button for Gate (at x = 25, y = 3) - high up, needs Wizard shoot
        World::getWorld().spawnActor<TriggerButton>(sf::Vector2f(25 * size, 3 * size), gate.get());

        // Platform next to the button so it makes visual sense
        World::getWorld().spawnActor<SolidBlock>(sf::Vector2f(25 * size, 4 * size), sf::Vector2f(size, size));

        // Gate at x = 9 (blocking the Ghost Barrier control) - wait, let's make the pressure plate disable the Ghost Barrier!
        // To do this, let's represent the GhostBarrier disablement. We can link a PressurePlate to a Gate. 
        // Let's spawn a Gate at x = 10 instead of a GhostBarrier, or let's make the PressurePlate open a gate that blocks the exit!
        // Yes, let's make the pressure plate at x = 28 open a second Gate at x = 30 which blocks the portal.
        auto exitGate = World::getWorld().spawnActor<Gate>(sf::Vector2f(29 * size, 6 * size), sf::Vector2f(size, 6 * size)).lock();
        
        // Pressure Plate (linked to exitGate) at x = 26
        World::getWorld().spawnActor<PressurePlate>(sf::Vector2f(25 * size, 11.75f * size), exitGate.get(), sf::Vector2f(1.5f * size, 20.0f));

        // Spawn Knight and Wizard at the start
        World::getWorld().spawnActor<PossessableKnight>(sf::Vector2f(3 * size, 10.0f * size));
        World::getWorld().spawnActor<PossessableWizard>(sf::Vector2f(6 * size, 10.0f * size));

        // Spawn Portal at the end
        World::getWorld().spawnActor<Portal>(sf::Vector2f(32 * size, 10.0f * size), [this]() {
            nextLevel();
        });

        // Spawn Player Ghost
        World::getWorld().spawnActor<Possessor>(sf::Vector2f(2 * size, 8 * size));
    }

    void buildLevel3() {
        float size = 80.0f;
        spawnBorderWalls(40, 16);

        // Spikes at the bottom from x = 12 to 24
        World::getWorld().spawnActor<Spikes>(sf::Vector2f(12 * size, 15.6f * size), sf::Vector2f(12 * size, 32.0f), [this]() {
            restartCurrentLevel();
        });

        // Steps to climb up to the starting platform (each step is 1 block higher)
        World::getWorld().spawnActor<SolidBlock>(sf::Vector2f(6 * size, 14 * size), sf::Vector2f(size, 2 * size));
        World::getWorld().spawnActor<SolidBlock>(sf::Vector2f(7 * size, 13 * size), sf::Vector2f(size, 3 * size));
        World::getWorld().spawnActor<SolidBlock>(sf::Vector2f(8 * size, 12 * size), sf::Vector2f(size, 4 * size));
        World::getWorld().spawnActor<SolidBlock>(sf::Vector2f(9 * size, 11 * size), sf::Vector2f(size, 5 * size));

        // High starting platform (floating, so characters can fall off to the right spikes)
        World::getWorld().spawnActor<SolidBlock>(sf::Vector2f(10 * size, 9 * size), sf::Vector2f(2 * size, size));
        
        // Target landing platform for Wizard glide (lowered to y = 12 so it's reachable)
        World::getWorld().spawnActor<SolidBlock>(sf::Vector2f(25 * size, 12 * size), sf::Vector2f(3 * size, 4 * size));

        // Gate at x = 10, y = 7 (blocks player on starting platform until button is shot)
        auto startGate = World::getWorld().spawnActor<Gate>(sf::Vector2f(10 * size, 7 * size), sf::Vector2f(size, 2 * size)).lock();
        
        // Button to open startGate is high up at x = 3, y = 3
        World::getWorld().spawnActor<TriggerButton>(sf::Vector2f(3 * size, 3 * size), startGate.get());
        World::getWorld().spawnActor<SolidBlock>(sf::Vector2f(3 * size, 4 * size), sf::Vector2f(size, size));

        // Gate blocking the portal at x = 34 (blocks floor to ceiling, so Ghost must open it!)
        auto portalGate = World::getWorld().spawnActor<Gate>(sf::Vector2f(34 * size, 0.0f), sf::Vector2f(size, 16 * size)).lock();

        // Pressure plate on the Wizard's landing platform (x = 26, y = 12) to open portalGate
        World::getWorld().spawnActor<PressurePlate>(sf::Vector2f(26 * size, 11.75f * size), portalGate.get(), sf::Vector2f(1.5f * size, 20.0f));

        // Spawn actors
        World::getWorld().spawnActor<PossessableKnight>(sf::Vector2f(2 * size, 14.0f * size));
        World::getWorld().spawnActor<PossessableWizard>(sf::Vector2f(5 * size, 14.0f * size));

        // Portal at the end
        World::getWorld().spawnActor<Portal>(sf::Vector2f(37 * size, 14.0f * size), [this]() {
            nextLevel();
        });

        // Player Ghost
        World::getWorld().spawnActor<Possessor>(sf::Vector2f(2 * size, 12 * size));
    }

    void buildWinScreen() {
        World::getWorld().spawnActor<EndScreen>([this]() {
            queueLevelLoad(0);
        });
    }

    void buildIntroScreen() {
        World::getWorld().spawnActor<StoryScreen>([this]() {
            queueLevelLoad(1);
        });
    }

    int _currentLevelIndex{ 0 };
    int _pendingLevelLoad{ -1 };
};
