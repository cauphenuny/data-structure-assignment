/// @file gui.cpp
/// @brief implementation of functions in gui.h

#include "gui/gui.hpp"

#include <SFML/Graphics.hpp>
#include <optional>
#include <cmath>
#include <iostream>
#include <vector>

GUIBase::GUIBase()
    : initialWidth(800),
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
      canvasView(window, windowSize, windowSize) {
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

void GUIBase::initEventListeners() {
    on<sf::Event::Closed>([this](const Event& event) {
        window.close();
    });

    on<sf::Event::Resized>([this](const Event& event) {
        const auto& resizeEvent = event->getIf<sf::Event::Resized>();
        windowSize = {static_cast<float>(resizeEvent->size.x), 
                      static_cast<float>(resizeEvent->size.y)};
        canvasView.update();
    });

    on<sf::Event::MouseWheelScrolled>([this](const Event& event) {
        const auto& scrollEvent = event->getIf<sf::Event::MouseWheelScrolled>();

        if (scrollEvent->wheel == sf::Mouse::Wheel::Vertical) {
            canvasView.setZoom(scrollEvent->delta, scrollEvent->position);
        }
    });

    on<sf::Event::MouseButtonPressed>([this](const Event& event) {
        const auto& mouseEvent = event->getIf<sf::Event::MouseButtonPressed>();
        
        if (mouseEvent->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePos(mouseEvent->position);
            dragMode = canvasView.getMouseDragMode(mousePos);
            lastMousePos = mousePos;
        }
    });

    on<sf::Event::MouseButtonReleased>([this](const Event& event) {
        dragMode = DragMode::NONE;
    });

    on<sf::Event::MouseMoved>([this](const Event& event) {
        const auto& mouseEvent = event->getIf<sf::Event::MouseMoved>();

        sf::Vector2f mousePos = {static_cast<float>(mouseEvent->position.x),
                                 static_cast<float>(mouseEvent->position.y)};

        if (dragMode == DragMode::NONE) {
            lastMousePos = mousePos;
            return;
        }

        sf::Vector2f delta = mousePos - lastMousePos;    

        float effectiveZoom = canvasView.getEffectiveZoom();
                    
        if (dragMode == DragMode::VIEW_DRAG) {
            // 视图拖动 - 根据缩放调整移动速度
            canvasView.pan(delta);
        } else {
            // 滚动条拖动 - 根据滚动比例计算偏移
            canvasView.scroll(delta, dragMode);
            
        }
        
        canvasView.update();
        lastMousePos = mousePos;
    });
}

void GUIBase::render() {
    // 计算当前有效缩放比例
    float effectiveZoom = canvasView.getEffectiveZoom();
    
    // 计算可视区域和内容区域
    float visibleWidth = windowSize.x / effectiveZoom;
    float visibleHeight = windowSize.y / effectiveZoom;
    
    // 计算最大偏移量
    float maxHOffset = std::max(0.0f, initialWidth - visibleWidth);
    float maxVOffset = std::max(0.0f, initialHeight - visibleHeight);

    window.clear(backgroundColor);
    
    window.setView(canvasView.getContentView());

    treeRenderer.render(window);
    window.draw(titleText);
    
    window.setView(canvasView.getUIView());
    
    canvasView.render();
    
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
        while (const std::optional event = window.pollEvent()) {        
            dispatchEvent<sf::Event::Closed,
                          sf::Event::Resized,
                          sf::Event::MouseWheelScrolled,
                          sf::Event::MouseButtonPressed,
                          sf::Event::MouseButtonReleased,
                          sf::Event::MouseMoved>
                (event);
        }

        // 更新动画 (简单的颜色变化)
        treeRenderer.update(0);
        
        canvasView.updateScrollbars();
        render();
    }
}
