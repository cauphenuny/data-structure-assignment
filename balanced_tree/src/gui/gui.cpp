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
      viewManager(window, windowSize, windowSize) {
    initWindow();
    initEventListeners();
}

void GUIBase::initWindow() {
    settings.antiAliasingLevel = 8;
    
    window.create(sf::VideoMode({initialWidth, initialHeight}), "Balanced Tree demo",
                  sf::Style::Default, sf::State::Windowed, settings);

    horizontalScrollbar.setSize({100.0f, scrollbarSize});
    horizontalScrollbarHandle.setSize({30.0f, scrollbarSize});
    verticalScrollbar.setSize({scrollbarSize, 100.0f});
    verticalScrollbarHandle.setSize({scrollbarSize, 30.0f});
    
    horizontalScrollbar.setFillColor(scrollbarColor);
    horizontalScrollbarHandle.setFillColor(scrollbarHandleColor);
    verticalScrollbar.setFillColor(scrollbarColor);
    verticalScrollbarHandle.setFillColor(scrollbarHandleColor);

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

void GUIBase::updateScrollbars() {
    float effectiveZoom = viewManager.getEffectiveZoom();
    
    // 计算可视区域和内容区域
    float visibleWidth = windowSize.x / effectiveZoom;
    float visibleHeight = windowSize.y / effectiveZoom;
    
    // 计算最大偏移量
    float maxHOffset = std::max(0.0f, initialWidth - visibleWidth);
    float maxVOffset = std::max(0.0f, initialHeight - visibleHeight);

    sf::Vector2f viewOffset = viewManager.getViewOffset();

    // 更新滚动条位置和大小
    float hScrollbarWidth = windowSize.x - scrollbarSize;
    float hHandleWidth = std::max(30.0f, hScrollbarWidth * (visibleWidth / initialWidth));
    float hHandlePosition = 0;
    
    if (maxHOffset > 0) {
        hHandlePosition = (viewOffset.x / maxHOffset) * (hScrollbarWidth - hHandleWidth);
    }
    
    horizontalScrollbar.setSize({hScrollbarWidth, scrollbarSize});
    horizontalScrollbar.setPosition({0, windowSize.y - scrollbarSize});
    horizontalScrollbarHandle.setSize({hHandleWidth, scrollbarSize});
    horizontalScrollbarHandle.setPosition({hHandlePosition, windowSize.y - scrollbarSize});
    
    // 更新垂直滚动条位置和大小
    float vScrollbarHeight = windowSize.y - scrollbarSize;
    float vHandleHeight = std::max(30.0f, vScrollbarHeight * (visibleHeight / initialHeight));
    float vHandlePosition = 0;
    
    if (maxVOffset > 0) {
        vHandlePosition = (viewOffset.y / maxVOffset) * (vScrollbarHeight - vHandleHeight);
    }
    
    verticalScrollbar.setSize({scrollbarSize, vScrollbarHeight});
    verticalScrollbar.setPosition({windowSize.x - scrollbarSize, 0});
    verticalScrollbarHandle.setSize({scrollbarSize, vHandleHeight});
    verticalScrollbarHandle.setPosition({windowSize.x - scrollbarSize, vHandlePosition});
}

void GUIBase::initEventListeners() {
    on<sf::Event::Closed>([this](const Event& event) {
        window.close();
    });

    on<sf::Event::Resized>([this](const Event& event) {
        const auto& resizeEvent = event->getIf<sf::Event::Resized>();
        windowSize = {static_cast<float>(resizeEvent->size.x), 
                      static_cast<float>(resizeEvent->size.y)};
        viewManager.update();
    });

    on<sf::Event::MouseWheelScrolled>([this](const Event& event) {
        const auto& scrollEvent = event->getIf<sf::Event::MouseWheelScrolled>();

        if (scrollEvent->wheel == sf::Mouse::Wheel::Vertical) {
            viewManager.setZoom(scrollEvent->delta, scrollEvent->position);
        }
    });

    on<sf::Event::MouseButtonPressed>([this](const Event& event) {
        const auto& mouseEvent = event->getIf<sf::Event::MouseButtonPressed>();
        
        if (mouseEvent->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePos(mouseEvent->position);
            
            // 检查是否点击在滚动条上
            if (horizontalScrollbarHandle.getGlobalBounds().contains(mousePos))
                dragMode = DragMode::HSCROLL_DRAG;
            else if (verticalScrollbarHandle.getGlobalBounds().contains(mousePos))
                dragMode = DragMode::VSCROLL_DRAG;
            else
                dragMode = DragMode::VIEW_DRAG;

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

        float effectiveZoom = viewManager.getEffectiveZoom();
                    
        if (dragMode == DragMode::VIEW_DRAG) {
            // 视图拖动 - 根据缩放调整移动速度
            viewManager.pan(delta);
        } else {
            // 滚动条拖动 - 根据滚动比例计算偏移
            bool isHorizontal = (dragMode == DragMode::HSCROLL_DRAG);

            float axisDelta   = isHorizontal ? delta.x : delta.y;
            float scrollbarSize = isHorizontal ? (windowSize.x - scrollbarSize)
                                               : (windowSize.y - scrollbarSize);
            float viewSize    = isHorizontal ? windowSize.x : windowSize.y;
            float contentSize = isHorizontal ? initialWidth : initialHeight;
            float handleSize  = isHorizontal ? horizontalScrollbarHandle.getSize().x
                                             : verticalScrollbarHandle.getSize().y;
            
            float visibleSize = viewSize / effectiveZoom;
            float maxOffset = std::max(0.0f, contentSize - visibleSize);

            float scrollRatio = axisDelta / (scrollbarSize - handleSize);

            sf::Vector2f viewOffset = viewManager.getViewOffset();
            
            float& offsetComponent = isHorizontal ? viewOffset.x : viewOffset.y;
            offsetComponent += scrollRatio * maxOffset;
            offsetComponent = std::clamp(offsetComponent, 0.0f, maxOffset);
            
        }
        
        viewManager.update();
        lastMousePos = mousePos;
    });
}

void GUIBase::render() {
    // 计算当前有效缩放比例
    float effectiveZoom = viewManager.getEffectiveZoom();
    
    // 计算可视区域和内容区域
    float visibleWidth = windowSize.x / effectiveZoom;
    float visibleHeight = windowSize.y / effectiveZoom;
    
    // 计算最大偏移量
    float maxHOffset = std::max(0.0f, initialWidth - visibleWidth);
    float maxVOffset = std::max(0.0f, initialHeight - visibleHeight);

    window.clear(backgroundColor);
    
    window.setView(viewManager.getContentView());

    treeRenderer.render(window);
    window.draw(titleText);
    
    window.setView(viewManager.getUIView());
    
    if (maxHOffset > 0) {
        window.draw(horizontalScrollbar);
        window.draw(horizontalScrollbarHandle);
    }
    
    if (maxVOffset > 0) {
        window.draw(verticalScrollbar);
        window.draw(verticalScrollbarHandle);
    }
    
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
        
        updateScrollbars();
        render();
    }
}
