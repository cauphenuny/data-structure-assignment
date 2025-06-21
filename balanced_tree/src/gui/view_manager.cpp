#include "gui/view_manager.hpp"
#include <iostream>

ViewManager::ViewManager(const sf::RenderWindow& window,
    sf::Vector2f initialSize, const sf::Vector2f& windowSize,
    float minZoom, float maxZoom) 
    : initialSize(initialSize),
      window(window),
      windowSize(windowSize),
      minZoom(minZoom),
      maxZoom(maxZoom),
      zoom(1.0f) {
    contentView.setCenter(initialSize / 2.f);
    contentView.setSize(initialSize);
    uiView = window.getDefaultView();
}

const sf::View& ViewManager::getContentView() const {
    return contentView;
}

const sf::View& ViewManager::getUIView() const {
    return uiView;
}

// 视图变换
void ViewManager::setZoom(float factor, sf::Vector2i focusPoint) {
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

void ViewManager::pan(sf::Vector2f delta) {
    viewOffset += -delta / effectiveZoom;
    contentView.setCenter(initialSize / 2.f + viewOffset);
}

// 自动适应内容
void ViewManager::fitContent(sf::FloatRect contentBounds, float padding) {
    if (contentBounds.size.x <= 0 || contentBounds.size.y <= 0)
        return;
    
    // 添加 padding 到内容边界
    contentBounds.position.x -= padding;
    contentBounds.position.y -= padding;
    contentBounds.size.x += 2 * padding;
    contentBounds.size.y += 2 * padding;
    
    // 计算适合内容的缩放级别
    float scaleX = windowSize.x / contentBounds.size.x;
    float scaleY = windowSize.y / contentBounds.size.y;
    float contentScale = std::min(scaleX, scaleY);
    
    zoom = std::clamp(contentScale, minZoom, maxZoom);
    
    // 计算内容中心，设置视图偏移使内容居中
    sf::Vector2f contentCenter =  contentBounds.position + contentBounds.size / 2.0f;
    viewOffset = contentCenter - initialSize / 2.0f;
    
    updateContentView();
}

float ViewManager::getEffectiveZoom() {
    float scaleX = windowSize.x / initialSize.x;
    float scaleY = windowSize.y / initialSize.y;
    effectiveZoom = zoom / std::min(scaleX, scaleY);
    return effectiveZoom;
}
sf::Vector2f ViewManager::getViewOffset() const {
    return viewOffset;
}

sf::Vector2f ViewManager::getWindowSize() const {
    return windowSize;
}

void ViewManager::update() {
    updateUIView();
    updateContentView();
}

void ViewManager::updateUIView() {
    uiView.setSize(windowSize);
    uiView.setCenter(windowSize / 2.f);
}

void ViewManager::updateContentView() {
    getEffectiveZoom();
    contentView.setSize(windowSize / effectiveZoom);
    contentView.setCenter(initialSize / 2.f + viewOffset);
}