#include "gui/views.hpp"
#include <iostream>

View::View(sf::RenderWindow& window,
    sf::Vector2f initialSize, const sf::Vector2f& viewSize,
    const sf::Vector2f& viewPosition, float minZoom, float maxZoom) 
    : initialSize(initialSize),
      window(window),
      viewSize(viewSize),
      viewPosition(viewPosition),
      minZoom(minZoom),
      maxZoom(maxZoom),
      zoom(1.0f),
      scrollManager(*this, this->initialSize.x, this->initialSize.y, viewSize) {
    contentView.setCenter(viewPosition + initialSize / 2.f);
    contentView.setSize(initialSize);
    uiView = contentView;
}

const sf::View& View::getContentView() const {
    return contentView;
}

const sf::View& View::getUIView() const {
    return uiView;
}

// 视图变换
void View::setZoom(float factor, sf::Vector2i focusPoint) {
    float zoomFactor = (factor > 0) ? 1.1f : 0.9f;
    float newZoom = zoom * zoomFactor;
    
    if (newZoom < minZoom || newZoom > maxZoom)
        return;
    
    // 获取鼠标在内容视图中的位置
    sf::Vector2f mousePos = window.mapPixelToCoords(
        focusPoint, contentView);
    
    zoom = newZoom;
    
    getEffectiveZoom();
    
    // 重新计算鼠标在内容视图中的位置
    sf::Vector2f newMousePos = window.mapPixelToCoords(
        focusPoint, contentView);
    
    // 调整视图偏移，使鼠标位置保持相对稳定
    viewOffset += (mousePos - newMousePos) * effectiveZoom;

    updateContentView();
}

void View::pan(sf::Vector2f delta) {
    viewOffset += -delta / effectiveZoom;
    contentView.setCenter(initialSize / 2.f + viewOffset);
}

void View::scroll(sf::Vector2f delta, DragMode dragMode) {
    bool isHorizontal = dragMode == DragMode::HSCROLL_DRAG;
    getEffectiveZoom();

    float scrollRatio = scrollManager.getScrollRatio(
        isHorizontal,
        isHorizontal ? delta.x : delta.y,
        effectiveZoom
    );

    sf::Vector2f maxOffset = getMaxOffset();
    float axisMaxOffset = isHorizontal ? maxOffset.x : maxOffset.y;

    float& offsetComponent = isHorizontal ? viewOffset.x : viewOffset.y;
    offsetComponent += scrollRatio * axisMaxOffset;
    offsetComponent = std::clamp(offsetComponent, 0.0f, axisMaxOffset);
}

// 自动适应内容
void View::fitContent(sf::FloatRect contentBounds, float padding) {
    if (contentBounds.size.x <= 0 || contentBounds.size.y <= 0)
        return;
    
    // 添加 padding 到内容边界
    contentBounds.position.x -= padding;
    contentBounds.position.y -= padding;
    contentBounds.size.x += 2 * padding;
    contentBounds.size.y += 2 * padding;
    
    // 计算适合内容的缩放级别
    float scaleX = viewSize.x / contentBounds.size.x;
    float scaleY = viewSize.y / contentBounds.size.y;
    float contentScale = std::min(scaleX, scaleY);
    
    zoom = std::clamp(contentScale, minZoom, maxZoom);
    
    // 计算内容中心，设置视图偏移使内容居中
    sf::Vector2f contentCenter =  contentBounds.position + contentBounds.size / 2.0f;
    viewOffset = contentCenter - initialSize / 2.0f;
    
    updateContentView();
}

float View::getEffectiveZoom() {
    float scaleX = viewSize.x / initialSize.x;
    float scaleY = viewSize.y / initialSize.y;
    effectiveZoom = zoom / std::min(scaleX, scaleY);
    return effectiveZoom;
}
sf::Vector2f& View::getViewOffset() {
    return viewOffset;
}

sf::Vector2f View::getWindowSize() const {
    return viewSize;
}

sf::Vector2f View::getMaxOffset() {
    getEffectiveZoom();
    sf::Vector2f visibleSize = viewSize / effectiveZoom;
    return {
        std::max(0.f, initialSize.x - visibleSize.x),
        std::max(0.f, initialSize.y - visibleSize.y),
    };
}

DragMode View::getMouseDragMode(sf::Vector2f mousePos) {
    return scrollManager.handleMousePress(mousePos);
}

sf::Vector2f View::getViewPosition() const {
    return viewPosition;
}

void View::setViewPosition(const sf::Vector2f& position) {
    const_cast<sf::Vector2f&>(viewPosition) = position;
    updateScrollbars();
}



void View::updateViews() {
    updateUIView();
    updateContentView();
}

void View::updateUIView() {
    uiView.setSize(viewSize);
    uiView.setCenter(viewSize / 2.f);
}

void View::updateContentView() {
    getEffectiveZoom();
    contentView.setSize(viewSize / effectiveZoom);
    contentView.setCenter(viewSize / 2.f + viewOffset);
}

void View::updateScrollbars() {
    scrollManager.update();
}

void View::render(sf::RenderTarget& target) {
    scrollManager.render(target);
}


CanvasView::CanvasView(sf::RenderWindow& window,
    sf::Vector2f initialSize, const sf::Vector2f& windowSize,
    const sf::Vector2f& viewPosition, float minZoom, float maxZoom)
    : View(window, initialSize, windowSize, viewPosition, minZoom, maxZoom) {
}

void CanvasView::setZoom(float factor, sf::Vector2i focusPoint) {
    float zoomFactor = (factor > 0) ? 1.1f : 0.9f;
    float newZoom = zoom * zoomFactor;
    
    if (newZoom < minZoom || newZoom > maxZoom)
        return;
    
    // 获取鼠标在内容视图中的位置
    sf::Vector2f mousePos = window.mapPixelToCoords(
        focusPoint, contentView);
    
    zoom = newZoom;
    
    getEffectiveZoom();
    
    // 重新计算鼠标在内容视图中的位置
    sf::Vector2f newMousePos = window.mapPixelToCoords(
        focusPoint, contentView);
        
    std::cout << "before: " << mousePos.x << ", " << mousePos.y << "\n";
    std::cout << " after: " << newMousePos.x << ", " << newMousePos.y << "\n";
    
    // 调整视图偏移，使鼠标位置保持相对稳定
    viewOffset += (mousePos - newMousePos) * effectiveZoom;

    updateContentView();
}

void CanvasView::pan(sf::Vector2f delta) {
    viewOffset += -delta / effectiveZoom;
    contentView.setCenter(initialSize / 2.f + viewOffset);
}

// void CanvasView::scroll(sf::Vector2f delta, DragMode dragMode) {
//     bool isHorizontal = dragMode == DragMode::HSCROLL_DRAG;
//     getEffectiveZoom();

//     float scrollRatio = scrollManager.getScrollRatio(
//         isHorizontal,
//         isHorizontal ? delta.x : delta.y,
//         effectiveZoom
//     );

//     sf::Vector2f maxOffset = getMaxOffset();
//     float axisMaxOffset = isHorizontal ? maxOffset.x : maxOffset.y;

//     float& offsetComponent = isHorizontal ? viewOffset.x : viewOffset.y;
//     offsetComponent += scrollRatio * axisMaxOffset;
//     offsetComponent = std::clamp(offsetComponent, 0.0f, axisMaxOffset);
// }