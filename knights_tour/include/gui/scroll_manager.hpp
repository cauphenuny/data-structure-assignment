#pragma once
#include <SFML/Graphics.hpp>
// #include "gui/view_manager.hpp"

enum class DragMode : uint8_t {
    NONE,
    VIEW_DRAG,
    HSCROLL_DRAG,
    VSCROLL_DRAG
};

class View;

class ScrollManager {
public:
    ScrollManager(View& view,
        const float& initialWidth, const float& initialHeight,
        const sf::Vector2f& windowSize, float scrollbarSize = 15.0f);
    
    // 更新滚动条
    void update();
    
    // 处理拖动
    DragMode handleMousePress(sf::Vector2f mousePos);
    void handleMouseDrag(sf::Vector2f delta, DragMode dragMode);
    
    // 渲染滚动条
    void render(sf::RenderTarget& target);
    
private:
    float scrollbarSize;
    sf::RectangleShape horizontalScrollbar;
    sf::RectangleShape horizontalScrollbarHandle;
    sf::RectangleShape verticalScrollbar;
    sf::RectangleShape verticalScrollbarHandle;
    
    sf::Color scrollbarColor;
    sf::Color scrollbarHandleColor;

    const float& initialWidth;
    const float& initialHeight;
    const sf::Vector2f& windowSize;

    View& view;
    
    // 计算可视比例
    float getScrollRatio(bool isHorizontal, float delta, float effectiveZoom);

    friend class View;
};