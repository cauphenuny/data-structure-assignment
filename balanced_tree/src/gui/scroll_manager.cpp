#include "gui/scroll_manager.hpp"
#include "gui/views.hpp"
#include <iostream>

ScrollManager::ScrollManager(View& view,
    const float& initialWidth, const float& initialHeight,
    const sf::Vector2f& windowSize, float scrollbarSize)
    : view(view),
      initialWidth(initialWidth),
      initialHeight(initialHeight),
      windowSize(windowSize),
      scrollbarSize(scrollbarSize),
      scrollbarColor(200, 200, 200),
      scrollbarHandleColor(150, 150, 150),
      horizontalScrollbar({100.0f, scrollbarSize}),
      horizontalScrollbarHandle({30.0f, scrollbarSize}),
      verticalScrollbar({scrollbarSize, 100.0f}),
      verticalScrollbarHandle({scrollbarSize, 30.0f}) {
    horizontalScrollbar.setFillColor(scrollbarColor);
    horizontalScrollbarHandle.setFillColor(scrollbarHandleColor);
    verticalScrollbar.setFillColor(scrollbarColor);
    verticalScrollbarHandle.setFillColor(scrollbarHandleColor);
}
    
// 更新滚动条
void ScrollManager::update() {
    float effectiveZoom = view.getEffectiveZoom();
    
    // 计算可视区域和内容区域
    float visibleWidth = windowSize.x / effectiveZoom;
    float visibleHeight = windowSize.y / effectiveZoom;
    
    sf::Vector2f maxOffset = view.getMaxOffset();
    
    // 水平滚动条（相对于视图坐标系）
    if (maxOffset.x > 0) {
        float hScrollbarWidth = windowSize.x - scrollbarSize;
        horizontalScrollbar.setSize({hScrollbarWidth, scrollbarSize});
        horizontalScrollbar.setPosition({0, windowSize.y - scrollbarSize});
        
        float hHandleWidth = std::max(20.0f, hScrollbarWidth * (visibleWidth / initialWidth));
        float hHandlePos = (view.viewOffset.x / maxOffset.x) * (hScrollbarWidth - hHandleWidth);
        
        horizontalScrollbarHandle.setSize({hHandleWidth, scrollbarSize});
        horizontalScrollbarHandle.setPosition({hHandlePos, windowSize.y - scrollbarSize});
    }
    
    // 垂直滚动条（相对于视图坐标系）
    if (maxOffset.y > 0) {
        float vScrollbarHeight = windowSize.y - scrollbarSize;
        verticalScrollbar.setSize({scrollbarSize, vScrollbarHeight});
        verticalScrollbar.setPosition({windowSize.x - scrollbarSize, 0});
        
        float vHandleHeight = std::max(20.0f, vScrollbarHeight * (visibleHeight / initialHeight));
        float vHandlePos = (view.viewOffset.y / maxOffset.y) * (vScrollbarHeight - vHandleHeight);
        
        verticalScrollbarHandle.setSize({scrollbarSize, vHandleHeight});
        verticalScrollbarHandle.setPosition({windowSize.x - scrollbarSize, vHandlePos});
    }
}


DragMode ScrollManager::handleMousePress(sf::Vector2f mousePos) {
    if (horizontalScrollbarHandle.getGlobalBounds().contains(mousePos))
        return DragMode::HSCROLL_DRAG;
    else if (verticalScrollbarHandle.getGlobalBounds().contains(mousePos))
        return DragMode::VSCROLL_DRAG;
    else
        return DragMode::VIEW_DRAG;
}

void ScrollManager::handleMouseDrag(sf::Vector2f delta, DragMode dragMode) {
    bool isHorizontal = (dragMode == DragMode::HSCROLL_DRAG);

    float axisDelta   = isHorizontal ? delta.x : delta.y;
    float scrollbarSize = isHorizontal ? (windowSize.x - scrollbarSize)
                                       : (windowSize.y - scrollbarSize);
    float viewSize    = isHorizontal ? windowSize.x : windowSize.y;
    float contentSize = isHorizontal ? view.initialSize.x : view.initialSize.y;
    float handleSize  = isHorizontal ? horizontalScrollbarHandle.getSize().x
                                     : verticalScrollbarHandle.getSize().y;
    
    float visibleSize = viewSize / view.effectiveZoom;
    float maxOffset = std::max(0.0f, contentSize - visibleSize);

    float scrollRatio = axisDelta / (scrollbarSize - handleSize);

    float& offsetComponent = isHorizontal ? view.viewOffset.x : view.viewOffset.y;
    offsetComponent += scrollRatio * maxOffset;
    offsetComponent = std::clamp(offsetComponent, 0.0f, maxOffset);
}

// 渲染滚动条
void ScrollManager::render(sf::RenderTarget& target) {
    auto [maxHOffset, maxVOffset] = view.getMaxOffset();

    if (maxHOffset > 0) {
        target.draw(horizontalScrollbar);
        target.draw(horizontalScrollbarHandle);
    }
    
    if (maxVOffset > 0) {
        target.draw(verticalScrollbar);
        target.draw(verticalScrollbarHandle);
    }
}

float ScrollManager::getScrollRatio(bool isHorizontal, float delta, float effectiveZoom) {
    float scrollbarSize = isHorizontal ? (windowSize.x - scrollbarSize)
                                       : (windowSize.y - scrollbarSize);
    float viewSize    = isHorizontal ? windowSize.x : windowSize.y;
    float contentSize = isHorizontal ? initialWidth : initialHeight;
    float handleSize  = isHorizontal ? horizontalScrollbarHandle.getSize().x
                                     : verticalScrollbarHandle.getSize().y;
    
    float visibleSize = viewSize / effectiveZoom;
    float maxOffset   = std::max(0.0f, contentSize - visibleSize);

    float scrollRatio = delta / (scrollbarSize - handleSize);
    return scrollRatio;
}