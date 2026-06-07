#pragma once
#include "Actor.h"
#include "SmartTextureLoader.h"
#include <SFML/Graphics.hpp>
#include <optional>
#include <cmath>
#include <functional>
#include <cstdint>

class StoryScreen : public Actor {
public:
    StoryScreen(std::function<void()> onStartGame) : Actor({ 0.0f, 0.0f }), _onStartGame(onStartGame) {
        // Load font with fallbacks
        if (_font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
            _hasFont = true;
        } else if (_font.openFromFile("C:/Windows/Fonts/consola.ttf")) {
            _hasFont = true;
        } else if (_font.openFromFile("C:/Windows/Fonts/calibri.ttf")) {
            _hasFont = true;
        }

        if (_hasFont) {
            _titleText.emplace(_font, "GEIST", 48);
            _titleText->setFillColor(sf::Color(255, 215, 0)); // Gold
            _titleText->setPosition({ 100.0f, 80.0f });

            // Multiline story text
            std::string story = 
                "For centuries, you were the Scourge of the Realm -\n"
                "an ancient, malevolent spirit capable of taking control of any living soul.\n\n"
                "But you were betrayed. Bound in silver chains, you were cast into\n"
                "the deepest depths of the Iron Dungeon, the kingdom's most secure fortress.\n\n"
                "Yet, chains of metal cannot hold a shadow forever.\n"
                "Your physical bindings have withered, but the dungeon guards remain.\n\n"
                "To escape, you must use your ultimate power: Possession.\n"
                "Cast your spirit into the minds of the unsuspecting heroes who trespass here.\n"
                "Command their bodies, exploit their unique skills, and turn the dungeon's\n"
                "own defenses against itself.\n\n"
                "Control is your key. The escape begins now...";
            
            _storyText.emplace(_font, story, 26);
            _storyText->setFillColor(sf::Color(200, 200, 220)); // Soft white/grey
            _storyText->setPosition({ 100.0f, 200.0f });

            _promptText.emplace(_font, "PRESS SPACE TO ESCAPE", 30);
            _promptText->setFillColor(sf::Color(0, 255, 255)); // Cyan
            _promptText->setPosition({ 100.0f, 850.0f });
        }

        try {
            const sf::Texture& texture = SmartTextureLoader::getTexture("../assets/ghost.png");
            _ghostSprite.emplace(texture);
            sf::FloatRect bounds = _ghostSprite->getLocalBounds();
            _ghostSprite->setOrigin(bounds.size / 2.0f);
            _ghostSprite->setScale({ 0.4f, 0.4f });
            _ghostSprite->setPosition({ 1400.0f, 450.0f });
        } catch (...) {
            // Silently fail if texture loader throws, so game doesn't crash
        }
    }

    void tick(float deltaTime) override {
        _totalTime += deltaTime;

        // Float the ghost sprite gently
        if (_ghostSprite) {
            float floatOffset = std::sin(_totalTime * 2.0f) * 20.0f;
            _ghostSprite->setPosition({ 1400.0f, 450.0f + floatOffset });
        }

        // Pulse prompt alpha
        if (_promptText) {
            float pulse = (std::sin(_totalTime * 3.0f) + 1.0f) / 2.0f;
            std::uint8_t alpha = static_cast<std::uint8_t>(100 + pulse * 155);
            _promptText->setFillColor(sf::Color(0, 255, 255, alpha));
        }

        // Transition to Level 1 on Space
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
            if (_onStartGame) {
                _onStartGame();
            }
        }

        Actor::tick(deltaTime);
    }

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        if (_ghostSprite) target.draw(*_ghostSprite, states);

        if (_titleText) target.draw(*_titleText, states);
        if (_storyText) target.draw(*_storyText, states);
        if (_promptText) target.draw(*_promptText, states);
    }

private:
    sf::Font _font;
    bool _hasFont{ false };
    std::optional<sf::Text> _titleText;
    std::optional<sf::Text> _storyText;
    std::optional<sf::Text> _promptText;

    std::optional<sf::Sprite> _ghostSprite;
    float _totalTime{ 0.0f };
    std::function<void()> _onStartGame;
};
