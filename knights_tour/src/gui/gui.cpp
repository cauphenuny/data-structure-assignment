/// @file gui.cpp
/// @brief implementation of functions in gui.h

#include "gui/gui.hpp"
#include "gui/chessboard_renderer.hpp"
#include "gui/control_panel.hpp"

#include <SFML/Graphics.hpp>
#include <iostream>

KnightsTourGUI::KnightsTourGUI()
    : initialWidth(1000),
      initialHeight(600),
      windowSize(initialWidth, initialHeight),
      backgroundColor(248, 248, 248),
      chessboardRenderer(font),
      controlPanel(font, chessboardRenderer),
      splitView(window, windowSize, lastMousePos) {
    
    std::cout << "GUI ready.\n";
    initWindow();
    initEventListeners();
}

void KnightsTourGUI::initWindow() {
    settings.antiAliasingLevel = 8;
    
    window.create(sf::VideoMode({initialWidth, initialHeight}), "Knights Tour Demo",
                  sf::Style::Default, sf::State::Windowed, settings);

    if (!font.openFromFile("assets/Menlo.ttf") &&
        !font.openFromFile("../assets/Menlo.ttf") && 
        !font.openFromFile("../../assets/Menlo.ttf")) {
        std::cerr << "Failed to load font." << std::endl;
    }
}

void KnightsTourGUI::initEventListeners() {
    on<sf::Event::Closed>([this](const Event& event) {
        window.close();
    });

    on<sf::Event::Resized>([this](const Event& event) {
        const auto& resizeEvent = event->getIf<sf::Event::Resized>();
        windowSize = {static_cast<float>(resizeEvent->size.x), 
                      static_cast<float>(resizeEvent->size.y)};
        splitView.updateViews();
    });

    on<sf::Event::MouseButtonPressed>([this](const Event& event) {
        const auto& mouseEvent = event->getIf<sf::Event::MouseButtonPressed>();
        
        if (mouseEvent->button == sf::Mouse::Button::Left) {
            sf::Vector2i mousePos(mouseEvent->position);
            
            // 获取鼠标在右侧面板中的位置
            float dividerX = windowSize.x * 0.7f;  // 分隔线位置
            if (mousePos.x > dividerX) {
                // 鼠标在右侧面板中
                sf::Vector2f relativePos(mousePos.x - dividerX, mousePos.y);
                controlPanel.handleMouseClick(relativePos);
            }
            
            lastMousePos = {
                static_cast<float>(mousePos.x),
                static_cast<float>(mousePos.y)};
        }
    });

    on<sf::Event::KeyPressed>([this](const Event& event) {
        const auto& keyEvent = event->getIf<sf::Event::KeyPressed>();
        controlPanel.handleKeyPress(keyEvent->code);
    });

    splitView.setLeftViewRenderer([this](sf::RenderWindow& window, const sf::View& view) {
        chessboardRenderer.render(window);
    });
    
    splitView.setRightViewRenderer([this](sf::RenderWindow& window, const sf::View& view) {
        controlPanel.render(window);
    });
}

void KnightsTourGUI::update(float deltaTime) {
    chessboardRenderer.update(deltaTime);
}

void KnightsTourGUI::render() {
    window.clear(backgroundColor);
    
    splitView.render(window);
    
    window.display();
}

void KnightsTourGUI::run() {
    while (window.isOpen()) {
        // 检查事件
        static sf::Clock deltaClock;
        float deltaTime = deltaClock.restart().asSeconds();

        while (const std::optional event = window.pollEvent()) {        
            dispatchEvent<sf::Event::Closed,
                          sf::Event::Resized,
                          sf::Event::MouseButtonPressed,
                          sf::Event::KeyPressed>
                (event);
        }

        update(deltaTime);
        render();
    }
}

// 主函数调用
void gui() {
    KnightsTourGUI gui;
    gui.run();
}
