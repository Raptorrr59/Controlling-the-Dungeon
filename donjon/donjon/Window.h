#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <memory>

class Window : public sf::RenderWindow {
public:
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    ~Window() = default;

    static void createInstance(unsigned int width, unsigned int height, const std::string& title) {
        _instance = std::unique_ptr<Window>(new Window(width, height, title));
    }

    static Window& getWindow() {
        return *_instance;
    }

private:
    Window(unsigned int width, unsigned int height, const std::string& title) {
        create(sf::VideoMode(sf::Vector2u(width, height)), title);
        setFramerateLimit(144);
    }

    inline static std::unique_ptr<Window> _instance = nullptr;
};