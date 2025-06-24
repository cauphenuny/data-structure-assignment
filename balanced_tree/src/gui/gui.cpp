/// @file gui.cpp
/// @brief implementation of functions in gui.h

#include "gui/gui.hpp"

#include <SFML/Graphics.hpp>
#include <optional>
#include <cmath>
#include <iostream>
#include <vector>

GUIBase::GUIBase()
    : initialWidth(1000),
      initialHeight(600),
      scrollbarSize(15.0f),
      windowSize(initialWidth, initialHeight),
      backgroundColor(248, 248, 248),
      lineColor(10, 136, 204),
      nodeOutlineColor(10, 136, 204),
      textColor(30, 30, 40),
      scrollbarColor(200, 200, 200),
      scrollbarHandleColor(150, 150, 150),
      dragMode(DragMode::NONE),
      titleText(font, ""),
      treeRenderer(font),
      splitView(window, windowSize, lastMousePos) {
    initWindow();
    initEventListeners();
}

void GUIBase::initWindow() {
    settings.antiAliasingLevel = 8;
    
    window.create(sf::VideoMode({initialWidth, initialHeight}), "Balanced Tree demo",
                  sf::Style::Default, sf::State::Windowed, settings);

    if (!font.openFromFile("assets/Menlo.ttf") &&
        !font.openFromFile("../assets/Menlo.ttf") && 
        !font.openFromFile("../../assets/Menlo.ttf")) {
        std::cerr << "Failed to load font." << std::endl;
    }

    titleText.setFont(font);
    titleText.setString("Balanced Tree Demo");
    titleText.setCharacterSize(24);
    titleText.setPosition({20, 20});
    titleText.setFillColor(textColor);
}

void GUIBase::initUI() {}

void GUIBase::initEventListeners() {
    on<sf::Event::Closed>([this](const Event& event) {
        window.close();
    });

    on<sf::Event::Resized>([this](const Event& event) {
        const auto& resizeEvent = event->getIf<sf::Event::Resized>();
        windowSize = {static_cast<float>(resizeEvent->size.x), 
                      static_cast<float>(resizeEvent->size.y)};
        splitView.updateViews();
    });

    on<sf::Event::MouseWheelScrolled>([this](const Event& event) {
        const auto& scrollEvent = event->getIf<sf::Event::MouseWheelScrolled>();

        if (scrollEvent->wheel == sf::Mouse::Wheel::Vertical) {
            sf::Vector2i mousePos = {scrollEvent->position.x, scrollEvent->position.y};
            splitView.handleMouseWheel(mousePos, scrollEvent->delta);
        }
    });

    on<sf::Event::MouseButtonPressed>([this](const Event& event) {
        const auto& mouseEvent = event->getIf<sf::Event::MouseButtonPressed>();
        
        if (mouseEvent->button == sf::Mouse::Button::Left) {
            sf::Vector2i mousePos(mouseEvent->position);
            dragMode = splitView.handleMousePress(mousePos);
            lastMousePos = {
                static_cast<float>(mousePos.x),
                static_cast<float>(mousePos.y)};
        }
    });

    on<sf::Event::MouseButtonReleased>([this](const Event& event) {
        dragMode = DragMode::NONE;
    });

    on<sf::Event::MouseMoved>([this](const Event& event) {
        const auto& mouseEvent = event->getIf<sf::Event::MouseMoved>();

        sf::Vector2i mousePos = {static_cast<float>(mouseEvent->position.x),
                                 static_cast<float>(mouseEvent->position.y)};

        splitView.handleMouseMove(mousePos, dragMode);
                                 
        splitView.updateViews();
        lastMousePos = {
            static_cast<float>(mousePos.x),
            static_cast<float>(mousePos.y)};
    });

    splitView.setLeftViewRenderer([this](sf::RenderWindow& window, const sf::View& view) {
        treeRenderer.render(window);
    });
    
    splitView.setRightViewRenderer([this](sf::RenderWindow& window, const sf::View& view) {});
}

void GUIBase::update(float deltaTime) {
    treeRenderer.update(deltaTime);
}

void GUIBase::render() {
    window.clear(backgroundColor);
    
    splitView.render(window);
    
    window.display();
}

void GUIBase::initTreeData() {
    treeRenderer.addNode((sf::Vector2f){400, 100}, 1);
    treeRenderer.addNode((sf::Vector2f){200, 200}, 2);
    treeRenderer.addNode((sf::Vector2f){600, 200}, 3);
    treeRenderer.addNode((sf::Vector2f){100, 300}, 4);
    treeRenderer.addNode((sf::Vector2f){300, 300}, 5);
    treeRenderer.addNode((sf::Vector2f){500, 300}, 6);
    treeRenderer.addNode((sf::Vector2f){700, 300}, 7);

    treeRenderer.addEdge(0, 1);
    treeRenderer.addEdge(0, 2);
    treeRenderer.addEdge(1, 3);
    treeRenderer.addEdge(1, 4);
    treeRenderer.addEdge(2, 5);
    treeRenderer.addEdge(2, 6);
}

void GUIBase::run() {
    initTreeData();

    while (window.isOpen()) {
        // 检查事件
        static sf::Clock deltaClock;
        float deltaTime = deltaClock.restart().asSeconds();

        while (const std::optional event = window.pollEvent()) {        
            dispatchEvent<sf::Event::Closed,
                          sf::Event::Resized,
                          sf::Event::MouseWheelScrolled,
                          sf::Event::MouseButtonPressed,
                          sf::Event::MouseButtonReleased,
                          sf::Event::MouseMoved,
                          sf::Event::KeyPressed,
                          sf::Event::TextEntered>
                (event);
        }

        update(deltaTime);
        render();
    }
}
