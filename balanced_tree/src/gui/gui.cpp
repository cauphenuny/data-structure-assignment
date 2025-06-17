/// @file gui.cpp
/// @brief implementation of functions in gui.h

#include "gui/gui.hpp"

#include <SFML/Graphics.hpp>
#include <optional>
#include <cmath>
#include <iostream>
#include <vector>

GUIBase::GUIBase() :
            initialWidth(800),
            initialHeight(600),
            scrollbarSize(15.0f),
            nodeRadius(25.0f),
            windowSize(initialWidth, initialHeight),
            backgroundColor(248, 248, 248),
            lineColor(10, 136, 204),
            nodeOutlineColor(10, 136, 204),
            textColor(30, 30, 40),
            scrollbarColor(200, 200, 200),
            scrollbarHandleColor(150, 150, 150),
            zoom(1.0f),
            minZoom(0.1f),
            maxZoom(5.0f),
            viewOffset(0.0f, 0.0f),
            dragMode(DragMode::NONE),
            titleText(font, "") {
    initWindow();
    initEventListeners();
}

void GUIBase::initWindow() {
    settings.antiAliasingLevel = 8;
    
    // 创建一个初始大小的窗口
    window.create(sf::VideoMode({initialWidth, initialHeight}), "Balanced Tree demo",
                  sf::Style::Default, sf::State::Windowed, settings);
    
    // 创建两个视图：一个用于内容，一个用于UI
    contentView.setCenter({initialWidth/2.f, initialHeight/2.f});
    contentView.setSize({initialWidth, initialHeight});
    uiView = window.getDefaultView();

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

inline float GUIBase::getEffectiveZoom() {
    // 设置内容视图缩放以填满窗口但保持比例
    // 移除视口设置，改为直接调整视图大小
    float scaleX = windowSize.x / initialWidth;
    float scaleY = windowSize.y / initialHeight;
    float effectiveZoom = zoom / std::min(scaleX, scaleY);
    return effectiveZoom;
}

inline void GUIBase::updateContentView() {
    contentView.setCenter({initialWidth/2.f + viewOffset.x, 
                          initialHeight/2.f + viewOffset.y});
}

inline void GUIBase::updateContentView(float effectiveZoom) {
    contentView.setSize(windowSize / effectiveZoom);
    contentView.setCenter({initialWidth/2.f + viewOffset.x, 
                          initialHeight/2.f + viewOffset.y});
}

/**
 * @brief Draws a line with feathered (soft) edges between two points
 * 
 * This function creates a line with smooth fading edges by rendering a triangle strip
 * with varying opacity. The inner part of the line has the specified color at full opacity,
 * while the outer edges fade to transparent, creating a soft, anti-aliased appearance.
 * 
 * @param window The render texture to draw on
 * @param start Starting point of the line
 * @param end Ending point of the line
 * @param color The color of the line
 * @param thickness The width of the solid part of the line (default: 1.0f)
 * @param feather The width of the fading edge on each side (default: 1.0f)
 */
void GUIBase::drawFeatheredLine(const sf::Vector2f& start, const sf::Vector2f& end,
                       sf::Color color, float thickness = 1.0f, float feather = 1.0f) {
    sf::Vector2f dir = end - start;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len == 0) return;
    dir /= len;
    sf::Vector2f normal(-dir.y, dir.x);

    thickness /= 2.f;
    sf::Vector2f half_n  = normal * thickness;
    sf::Vector2f outer_n = normal * (thickness + feather);

    sf::Color transparent = color;
    transparent.a = 0;

    sf::VertexArray triangles(sf::PrimitiveType::TriangleStrip, 8);

    triangles[0] = {start + outer_n,  transparent};
    triangles[1] = { end  + outer_n,  transparent};
    triangles[2] = {start + half_n,   color      };
    triangles[3] = { end  + half_n,   color      };
    triangles[4] = {start - half_n,   color      };
    triangles[5] = { end  - half_n,   color      };
    triangles[6] = {start - outer_n,  transparent};
    triangles[7] = { end  - outer_n,  transparent};

    window.draw(triangles);
}

void GUIBase::updateScrollbars() {
    // 计算当前有效缩放比例
    float scaleX = windowSize.x / initialWidth;
    float scaleY = windowSize.y / initialHeight;
    float effectiveZoom = zoom / std::min(scaleX, scaleY);
    
    // 计算可视区域和内容区域
    float visibleWidth = windowSize.x / effectiveZoom;
    float visibleHeight = windowSize.y / effectiveZoom;
    
    // 计算最大偏移量
    float maxHOffset = std::max(0.0f, initialWidth - visibleWidth);
    float maxVOffset = std::max(0.0f, initialHeight - visibleHeight);

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
        
        // 更新UI视图以匹配新窗口
        uiView.setSize(windowSize);
        uiView.setCenter(windowSize / 2.f);
        
        float effectiveZoom = getEffectiveZoom();
        
        updateContentView(effectiveZoom);
    });

    on<sf::Event::MouseWheelScrolled>([this](const Event& event) {
        const auto& scrollEvent = event->getIf<sf::Event::MouseWheelScrolled>();

        if (scrollEvent->wheel == sf::Mouse::Wheel::Vertical) {
            float zoomFactor = (scrollEvent->delta > 0) ? 1.1f : 0.9f;
            float newZoom = zoom * zoomFactor;
            
            // 限制缩放范围
            if (newZoom < minZoom || newZoom > maxZoom)
                return;
            
            // 获取鼠标在内容视图中的位置
            sf::Vector2f mousePos = window.mapPixelToCoords(
                scrollEvent->position, contentView);
            
            // 更新缩放
            zoom = newZoom;
            
            float effectiveZoom = getEffectiveZoom();
            
            // 调整内容视图大小
            contentView.setSize(windowSize / effectiveZoom);
            
            // 重新计算鼠标在内容视图中的位置
            sf::Vector2f newMousePos = window.mapPixelToCoords(
                scrollEvent->position, contentView);
            
            // 调整视图偏移，使鼠标位置保持相对稳定
            viewOffset += (mousePos - newMousePos) * effectiveZoom;
            updateContentView();
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
        float effectiveZoom = getEffectiveZoom();
                    
        if (dragMode == DragMode::VIEW_DRAG) {
            // 视图拖动 - 根据缩放调整移动速度
            viewOffset += -delta / effectiveZoom;
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
            
            float& offsetComponent = isHorizontal ? viewOffset.x : viewOffset.y;
            offsetComponent += scrollRatio * maxOffset;
            offsetComponent = std::clamp(offsetComponent, 0.0f, maxOffset);
            
        }
        
        // 更新视图中心点
        updateContentView();
        lastMousePos = mousePos;
    });
}

void GUIBase::render() {
    // 计算当前有效缩放比例
    float scaleX = windowSize.x / initialWidth;
    float scaleY = windowSize.y / initialHeight;
    float effectiveZoom = zoom / std::min(scaleX, scaleY);
    
    // 计算可视区域和内容区域
    float visibleWidth = windowSize.x / effectiveZoom;
    float visibleHeight = windowSize.y / effectiveZoom;
    
    // 计算最大偏移量
    float maxHOffset = std::max(0.0f, initialWidth - visibleWidth);
    float maxVOffset = std::max(0.0f, initialHeight - visibleHeight);

    // 清屏
    window.clear(backgroundColor);
    
    // 绘制内容
    window.setView(contentView);
    
    // 直接绘制边
    for (auto& treeEdge: treeEdges) {
        drawFeatheredLine(
            treeNodes[treeEdge.from].position,
            treeNodes[treeEdge.to].position,
            lineColor, 1.5f, 1.0f);
    }
    
    // 绘制节点和标签
    for (auto& node : nodes) {
        window.draw(node);
    }
    
    for (auto& label : nodeLabels) {
        window.draw(label);
    }
    
    window.draw(titleText);
    
    // 切换到UI视图绘制滚动条
    window.setView(uiView);
    
    // 仅当内容超出可视区域时显示滚动条
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

void GUIBase::run() {
    edges.reserve(20);

    sf::Vector2f nodeCircle(nodeRadius, nodeRadius);

    treeNodes.emplace_back((sf::Vector2f){400 - nodeRadius, 100 - nodeRadius}, 1);
    treeNodes.emplace_back((sf::Vector2f){200 - nodeRadius, 200 - nodeRadius}, 2);
    treeNodes.emplace_back((sf::Vector2f){600 - nodeRadius, 200 - nodeRadius}, 3);
    treeNodes.emplace_back((sf::Vector2f){100 - nodeRadius, 300 - nodeRadius}, 4);
    treeNodes.emplace_back((sf::Vector2f){300 - nodeRadius, 300 - nodeRadius}, 5);
    treeNodes.emplace_back((sf::Vector2f){500 - nodeRadius, 300 - nodeRadius}, 6);
    treeNodes.emplace_back((sf::Vector2f){700 - nodeRadius, 300 - nodeRadius}, 7);

    treeEdges.emplace_back(0, 1);
    treeEdges.emplace_back(0, 2);
    treeEdges.emplace_back(1, 3);
    treeEdges.emplace_back(1, 4);
    treeEdges.emplace_back(2, 5);
    treeEdges.emplace_back(2, 6);

    // 添加节点
    for (auto& treeNode: treeNodes) {
        sf::CircleShape circle(nodeRadius);
        circle.setPointCount(120); // 增加点数，默认是30
        circle.setFillColor(sf::Color::White);
        circle.setOutlineColor(nodeOutlineColor);
        circle.setOutlineThickness(2);
        
        sf::Text label(font, std::to_string(treeNode.value), 20);
        label.setFillColor(textColor);

        sf::Vector2f nodePosition = treeNode.position - nodeCircle;
        sf::Vector2f labelBound   = label.getLocalBounds().size;
        
        circle.setPosition(nodePosition);
        label.setPosition(nodePosition - labelBound/2.0f + sf::Vector2f(20, 20));
        // std::cout << (std::string)label.getString() << " ";

        nodes.push_back(circle);
        nodeLabels.push_back(label);
    }

    for (auto& treeEdge: treeEdges) {
        // Get source and target node indices
        
        // sf::Vector2f fromPos = treeNodes[fromIdx].position + sf::Vector2f(nodeRadius, nodeRadius);
        // sf::Vector2f toPos = treeNodes[toIdx].position + sf::Vector2f(nodeRadius, nodeRadius);

        // Add the edge (two vertices for start and end)
        edges.emplace_back(treeNodes[treeEdge.from].position);
        edges.emplace_back(treeNodes[treeEdge.to].position);
    }
    
    // 主循环
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
        float time = clock.getElapsedTime().asSeconds();
        for (size_t i = 0; i < nodes.size(); i++) {
            // 周期性更改节点颜色
            int r = 100 + static_cast<int>(50 * std::sin(time + i));
            int g = 150 + static_cast<int>(50 * std::sin(time * 0.7f + i));
            int b = 250 + static_cast<int>(50 * std::sin(time * 0.5f + i));
            nodes[i].setFillColor(sf::Color(r, g, b));
        }
        
        updateScrollbars();
        render();
    }
}
