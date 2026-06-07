#include <SFML/Graphics.hpp>
#include "Window.h"
#include "World.h"
#include "LevelManager.h"

int main()
{
    // Create window
    Window::createInstance(1920, 1080, "Geist");
    Window &window = Window::getWindow();

    World& world = World::getWorld();

    // Disable showing debug colliders if desired, or keep them on for visibility
    // BoxColliderComponent::bShowDebugColliders = false;

    // Load first level (Level 0 is the Intro Story Screen)
    LevelManager::getInstance().loadLevel(0);

    sf::Clock clock;
    while (window.isOpen())
    {
        LevelManager::getInstance().processPendingLevelLoad();

        float deltaTime = clock.restart().asSeconds();
        if (deltaTime > 0.05f) {
            deltaTime = 0.05f; // Cap delta time to prevent physics glitches
        }

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                if (auto activeCam = CameraManager::getInstance().getActiveCamera()) {
                    sf::Vector2f newSize{ static_cast<float>(resized->size.x), static_cast<float>(resized->size.y) };
                    activeCam->getModifiableView().setSize(newSize);
                }
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::R) {
                    LevelManager::getInstance().restartCurrentLevel();
                }
            }
        }

        world.tick(deltaTime);

        window.clear(sf::Color(15, 15, 20)); // Nice dark space background
        world.draw(window);
        window.display();
    }
}