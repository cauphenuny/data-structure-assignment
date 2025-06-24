#pragma once

#include <SFML/Graphics.hpp>
#include "algorithm.h"
#include "gui/chessboard_renderer.hpp"
#include <functional>
#include <vector>
#include <string>

class Button {
public:
    Button(const sf::Font& font, const std::string& text,
        sf::Vector2f position, sf::Vector2f size);
    void render(sf::RenderWindow& window);
    bool isMouseOver(sf::Vector2f mousePos) const;
    std::function<void()> onClick;
    
private:
    sf::RectangleShape shape;
    sf::Text text;
};

class TextField {
public:
    TextField(const sf::Font& font, const std::string& label, sf::Vector2f position, sf::Vector2f size);
    void render(sf::RenderWindow& window);
    bool isMouseOver(sf::Vector2f mousePos) const;
    std::string getValue() const;
    void setValue(const std::string& value);
    
private:
    sf::RectangleShape shape;
    sf::Text labelText;
    sf::Text valueText;
    std::string value;
};

class ControlPanel {
public:
    ControlPanel(sf::Font& font, ChessboardRenderer& renderer);
    
    void render(sf::RenderWindow& window);
    void handleMouseClick(sf::Vector2f mousePos);
    void handleKeyPress(sf::Keyboard::Key key);
    
private:
    void createControls();
    
    sf::Font& font;
    ChessboardRenderer& renderer;
    
    std::vector<Button> buttons;
    std::vector<TextField> textFields;
    
    TextField* activeTextField;
    
    // 控制面板状态
    int startX;
    int startY;
    Algorithm algorithm;
    float speed;
};