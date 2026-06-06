#include <SFML/Graphics.hpp>
#include "Window.h"
#include "World.h"
#include "Player.h"
#include "Block.h"

int main()
{
    Window::createInstance(1920, 1080, "Dungeon Engine - DEBUG");
    Window &window = Window::getWindow();

    World& world = World::getWorld();

    auto p = World::getWorld().spawnActor<Player>(sf::Vector2f{ 640.0f, 360.0f });
    auto b = World::getWorld().spawnActor<Block>(sf::Vector2f{ 640.0f, 360.0f });

    if (auto pv = p.lock()) {
        pv->getComponent<TransformComponent>()->setPosition(sf::Vector2f{ 100.0f, 100.0f });
    }

    sf::Clock clock;
    while (window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        world.tick(deltaTime);

        window.clear();
        world.draw(window);
        window.display();
    }
}