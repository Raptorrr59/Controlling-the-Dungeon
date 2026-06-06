#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

class Window : public sf::RenderWindow {
public:
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    inline void close() {
        sf::RenderWindow::close();
    }

    inline static [[nodiscard]] Window& getWindow() {
        static Window window;
        return window;
    }

private:
    Window() : sf::RenderWindow(sf::VideoMode({ 800, 600 }), "My window") {}
    ~Window() = default;
};