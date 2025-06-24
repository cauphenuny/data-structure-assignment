#include "gui/control_panel.hpp"
#include <iostream>

Button::Button(const sf::Font& font, const std::string& buttonText, 
              sf::Vector2f position, sf::Vector2f size)
    : shape(size), text(font) {
    
    shape.setPosition(position);
    shape.setFillColor(sf::Color(220, 220, 220));
    shape.setOutlineThickness(1);
    shape.setOutlineColor(sf::Color(100, 100, 100));
    
    text.setFont(font);
    text.setString(buttonText);
    text.setCharacterSize(16);
    text.setFillColor(sf::Color::Black);
    
    // 居中文本
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin({textBounds.position.x + textBounds.size.x/2.0f,
                    textBounds.position.y + textBounds.size.y/2.0f});
    text.setPosition({position.x + size.x/2.0f, position.y + size.y/2.0f});
}

void Button::render(sf::RenderWindow& window) {
    window.draw(shape);
    window.draw(text);
}

bool Button::isMouseOver(sf::Vector2f mousePos) const {
    return shape.getGlobalBounds().contains(mousePos);
}

TextField::TextField(const sf::Font& font, const std::string& label, 
                    sf::Vector2f position, sf::Vector2f size)
    : shape(size), value("0"), labelText(font), valueText(font) {
    
    shape.setPosition(position);
    shape.setFillColor(sf::Color::White);
    shape.setOutlineThickness(1);
    shape.setOutlineColor(sf::Color(100, 100, 100));
    
    labelText.setFont(font);
    labelText.setString(label);
    labelText.setCharacterSize(14);
    labelText.setFillColor(sf::Color::Black);
    labelText.setPosition({position.x, position.y - 20});
    
    valueText.setFont(font);
    valueText.setString(value);
    valueText.setCharacterSize(16);
    valueText.setFillColor(sf::Color::Black);
    valueText.setPosition({position.x + 5, position.y + 5});
}

void TextField::render(sf::RenderWindow& window) {
    window.draw(labelText);
    window.draw(shape);
    window.draw(valueText);
}

bool TextField::isMouseOver(sf::Vector2f mousePos) const {
    return shape.getGlobalBounds().contains(mousePos);
}

std::string TextField::getValue() const {
    return value;
}

void TextField::setValue(const std::string& newValue) {
    value = newValue;
    valueText.setString(value);
}

ControlPanel::ControlPanel(sf::Font& font, ChessboardRenderer& renderer)
    : font(font), 
      renderer(renderer), 
      activeTextField(nullptr),
      startX(0),
      startY(0),
      algorithm(Algorithm::HEURISTIC),
      speed(1.0f) {
    createControls();
}

void ControlPanel::createControls() {
    // 起始位置X输入框
    textFields.push_back(TextField(font, "Start X (0-7):", sf::Vector2f(20, 50), sf::Vector2f(60, 30)));
    
    // 起始位置Y输入框
    textFields.push_back(TextField(font, "Start Y (0-7):", sf::Vector2f(20, 120), sf::Vector2f(60, 30)));
    
    // 速度输入框
    textFields.push_back(TextField(font, "Speed (0.1-5):", sf::Vector2f(20, 190), sf::Vector2f(60, 30)));
    textFields[2].setValue("1.0");

    std::cout << "yes\n";
    
    // 运行按钮 - 暴力算法
    buttons.push_back(Button(font, "Run (Brute Force)", sf::Vector2f(20, 260), sf::Vector2f(180, 40)));
    buttons[0].onClick = [this]() {
        try {
            startX = std::stoi(textFields[0].getValue());
            startY = std::stoi(textFields[1].getValue());
            speed = std::stof(textFields[2].getValue());
            
            renderer.setSpeed(speed);
            renderer.setStartPosition(startX, startY);
            renderer.runAlgorithm(Algorithm::BRUTE_FORCE);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    };
    
    // 运行按钮 - 启发式算法
    buttons.push_back(Button(font, "Run (Heuristic)", sf::Vector2f(20, 310), sf::Vector2f(180, 40)));
    buttons[1].onClick = [this]() {
        try {
            startX = std::stoi(textFields[0].getValue());
            startY = std::stoi(textFields[1].getValue());
            speed = std::stof(textFields[2].getValue());
            
            renderer.setSpeed(speed);
            renderer.setStartPosition(startX, startY);
            renderer.runAlgorithm(Algorithm::HEURISTIC);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    };
    
    // 运行按钮 - 增强型启发式算法
    buttons.push_back(Button(font, "Run (Enhanced)", sf::Vector2f(20, 360), sf::Vector2f(180, 40)));
    buttons[2].onClick = [this]() {
        try {
            startX = std::stoi(textFields[0].getValue());
            startY = std::stoi(textFields[1].getValue());
            speed = std::stof(textFields[2].getValue());
            
            renderer.setSpeed(speed);
            renderer.setStartPosition(startX, startY);
            renderer.runAlgorithm(Algorithm::HEURISTIC_INHANCER);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    };
    
    // 暂停/继续按钮
    buttons.push_back(Button(font, "Pause/Resume", sf::Vector2f(20, 410), sf::Vector2f(180, 40)));
    buttons[3].onClick = [this]() {
        renderer.togglePause();
    };
    
    // 下一步按钮
    buttons.push_back(Button(font, "Next Step", sf::Vector2f(20, 460), sf::Vector2f(180, 40)));
    buttons[4].onClick = [this]() {
        renderer.nextStep();
    };
    
    // 重置按钮
    buttons.push_back(Button(font, "Reset", sf::Vector2f(20, 510), sf::Vector2f(180, 40)));
    buttons[5].onClick = [this]() {
        renderer.reset();
    };
}

void ControlPanel::render(sf::RenderWindow& window) {
    // 绘制标题
    sf::Text title(font);
    title.setString("Knights Tour Control");
    title.setCharacterSize(20);
    title.setFillColor(sf::Color::Black);
    title.setPosition({20, 10});
    window.draw(title);
    
    // 绘制所有控制元素
    for (auto& textField : textFields) {
        textField.render(window);
    }
    
    for (auto& button : buttons) {
        button.render(window);
    }
}

void ControlPanel::handleMouseClick(sf::Vector2f mousePos) {
    // 检查是否点击了按钮
    for (auto& button : buttons) {
        if (button.isMouseOver(mousePos) && button.onClick) {
            button.onClick();
            return;
        }
    }
    
    // 检查是否点击了文本框
    activeTextField = nullptr;
    for (auto& textField : textFields) {
        if (textField.isMouseOver(mousePos)) {
            activeTextField = &textField;
            return;
        }
    }
}

void ControlPanel::handleKeyPress(sf::Keyboard::Key key) {
    if (!activeTextField) return;
    
    std::string value = activeTextField->getValue();
    
    if (key == sf::Keyboard::Key::Backspace) {
        if (!value.empty()) {
            value.pop_back();
        }
    } else if (key >= sf::Keyboard::Key::Num0 && key <= sf::Keyboard::Key::Num9) {
        value += std::to_string(static_cast<uint8_t>(key) - static_cast<uint8_t>(sf::Keyboard::Key::Num0));
    } else if (key >= sf::Keyboard::Key::Numpad0 && key <= sf::Keyboard::Key::Numpad9) {
        value += std::to_string(static_cast<uint8_t>(key) - static_cast<uint8_t>(sf::Keyboard::Key::Numpad0));
    } else if (key == sf::Keyboard::Key::Period) {
        // 只允许输入一个小数点
        if (value.find('.') == std::string::npos) {
            value += '.';
        }
    }
    
    activeTextField->setValue(value);
}