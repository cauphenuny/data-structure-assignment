#include "gui/split_view.hpp"
#include <iostream>

SplitView::SplitView(sf::RenderWindow& window, const sf::Vector2f& windowSize,
    sf::Vector2f& lastMousePos)
    : window(window),
      windowSize(windowSize),
      lastMousePos(lastMousePos),
      dividerPosition(0.7f),
      dividerWidth(2.0f),
      isDraggingDivider(false),
      leftViewSize({windowSize.x * dividerPosition - dividerWidth/2, windowSize.y}),
      rightViewSize({windowSize.x * (1 - dividerPosition) - dividerWidth/2, windowSize.y}),
      leftViewPosition({0, 0}),
      rightViewPosition({windowSize.x * dividerPosition, 0}),
      leftView(window, leftViewSize, leftViewSize, leftViewPosition),
      rightView(window, rightViewSize, rightViewSize, rightViewPosition) {

    divider.setFillColor(sf::Color(180, 180, 180));
    
    // 初始化分隔线
    divider.setSize({dividerWidth, windowSize.y});
    
    // 初始化背景
    leftBackground.setFillColor(sf::Color(253, 253, 253));
    rightBackground.setFillColor(sf::Color(245, 245, 245));
    
    updateDividerPosition(dividerPosition);
    updateBackgrounds();
}

void SplitView::updateBackgrounds() {
    float leftWidth = windowSize.x * dividerPosition - dividerWidth/2;
    float rightWidth = windowSize.x - leftWidth - dividerWidth;
    
    // 设置左侧背景
    leftBackground.setSize({leftWidth, windowSize.y});
    leftBackground.setPosition({0, 0});
    
    // 设置右侧背景
    rightBackground.setSize({rightWidth, windowSize.y});
    rightBackground.setPosition({leftWidth + dividerWidth, 0});
}

void SplitView::updateViews() {
    float leftWidth = windowSize.x * dividerPosition - dividerWidth/2;
    float rightWidth = windowSize.x - leftWidth - dividerWidth;
    
    // 更新视图大小
    leftViewSize = {leftWidth, windowSize.y};
    rightViewSize = {rightWidth, windowSize.y};
    
    // 更新视图位置
    leftViewPosition = {0, 0};
    rightViewPosition = {leftWidth + dividerWidth, 0};
    
    // 设置视图位置和大小
    leftView.setViewPosition(leftViewPosition);
    rightView.setViewPosition(rightViewPosition);
    
    // 更新两个视图
    leftView.updateViews();
    rightView.updateViews();
    
    // 更新滚动条
    leftView.updateScrollbars();
    rightView.updateScrollbars();
    
    // 更新背景
    updateBackgrounds();
}

void SplitView::updateDividerPosition(float newPosition) {
    // 限制分隔线位置在合理范围内
    dividerPosition = std::clamp(newPosition, 0.1f, 0.9f);
    
    float xPos = windowSize.x * dividerPosition - dividerWidth/2;
    divider.setPosition({xPos, 0});
    
    updateViews();
}

void SplitView::setCursorType(sf::Cursor::Type type) {
    sf::Cursor cursor(type);
    window.setMouseCursor(cursor);
}

bool SplitView::isMouseOverDivider(sf::Vector2i mousePos) {
    return divider.getGlobalBounds().contains(sf::Vector2f(mousePos));
}

DragMode SplitView::handleMousePress(sf::Vector2i mousePos) {
    // 检查是否点击在分隔线上
    if (isMouseOverDivider(mousePos)) {
        isDraggingDivider = true;
    } else {
        // 根据鼠标位置决定将事件转发给哪个视图
        sf::Vector2f mouseFloatPos(mousePos);
        
        float dividerX = windowSize.x * dividerPosition;
        if (mouseFloatPos.x < dividerX - dividerWidth/2) {
            // 鼠标在左侧视图，转发事件给左侧视图
            sf::Vector2f relativePos = mouseFloatPos - leftViewPosition;
            DragMode leftDragMode = leftView.getMouseDragMode(mouseFloatPos);
            return leftDragMode;
        } else if (mouseFloatPos.x > dividerX + dividerWidth/2) {
            sf::Vector2f relativePos = mouseFloatPos - rightViewPosition;
            DragMode rightDragMode = rightView.getMouseDragMode(mouseFloatPos);
            return rightDragMode;
        }
    }
}

void SplitView::handleMouseRelease() {
    isDraggingDivider = false;
}

void SplitView::handleMouseMove(sf::Vector2i mousePos, DragMode dragMode) {
    if (isDraggingDivider) {
        // 更新分隔线位置
        float newPosition = static_cast<float>(mousePos.x) / windowSize.x;
        updateDividerPosition(newPosition);
    } else if (isMouseOverDivider(mousePos)) {
        // 鼠标悬停在分隔线上
        // setCursorType(sf::Cursor::Type::SizeHorizontal);
    } else {        
        // 转发鼠标移动事件给相应的视图
        // 类似于handleMousePress中的逻辑
        sf::Vector2f mouseFloatPos(mousePos);
        float dividerX = windowSize.x * dividerPosition;
        
        if (dragMode == DragMode::VIEW_DRAG) {
            // 处理视图拖动
            if (mouseFloatPos.x < dividerX - dividerWidth/2) {
                sf::Vector2f delta = mouseFloatPos - lastMousePos;
                leftView.pan(delta);
            }
        } else if (dragMode == DragMode::HSCROLL_DRAG || 
                   dragMode == DragMode::VSCROLL_DRAG) {
            // 处理滚动条拖动
            sf::Vector2f delta = mouseFloatPos - lastMousePos;
            if (mouseFloatPos.x < dividerX - dividerWidth/2) {
                leftView.scroll(delta, dragMode);
            } else if (mouseFloatPos.x > dividerX + dividerWidth/2) {
                rightView.scroll(delta, dragMode);
            }
        }
        
        lastMousePos = mouseFloatPos;
    }
}

void SplitView::handleMouseWheel(sf::Vector2i mousePos, float delta) {
    // 根据鼠标位置决定将滚轮事件转发给哪个视图
    sf::Vector2f mouseFloatPos(mousePos);
    std::cout << "Hello\n";
    float dividerX = windowSize.x * dividerPosition;
    if (mouseFloatPos.x < dividerX - dividerWidth/2) {
        // 转发给左侧视图
        std::cout << "left\n";
        leftView.setZoom(delta, mousePos);
    } else if (mouseFloatPos.x > dividerX + dividerWidth/2) {
        // 转发给右侧视图
        std::cout << "right\n";
        rightView.setZoom(delta, mousePos);
    }
}

void SplitView::render(sf::RenderTarget& target) {
    // 恢复默认视图绘制背景
    window.setView(window.getDefaultView());
    
    // 绘制背景
    target.draw(leftBackground);
    target.draw(rightBackground);
    
    // 计算视口
    float leftViewportWidth = dividerPosition - dividerWidth / (2.0f * windowSize.x);
    float rightViewportWidth = (1.0f - dividerPosition) - dividerWidth / (2.0f * windowSize.x);
    float rightViewportLeft = dividerPosition + dividerWidth / (2.0f * windowSize.x);
    
    // 渲染左侧视图
    sf::View leftContentView = leftView.getContentView();
    leftContentView.setViewport(sf::FloatRect({0.0f, 0.0f}, {leftViewportWidth, 1.0f}));
    window.setView(leftContentView);
    
    if (leftViewRenderer) {
        leftViewRenderer(window, leftContentView);
    }
    
    // 渲染右侧视图
    sf::View rightContentView = rightView.getContentView();
    rightContentView.setViewport(sf::FloatRect({rightViewportLeft, 0.0f}, {rightViewportWidth, 1.0f}));
    window.setView(rightContentView);
    
    if (rightViewRenderer) {
        rightViewRenderer(window, rightContentView);
    }
    
    // 恢复默认视图绘制UI元素（分隔线和滚动条）
    window.setView(window.getDefaultView());
    
    // 绘制分隔线
    target.draw(divider);
    
    // 渲染UI元素（滚动条等）
    // renderScrollbars(target);
}

void SplitView::renderScrollbars(sf::RenderTarget& target) {
    // 渲染左侧视图的滚动条
    sf::View leftUIView = leftView.getUIView();
    leftUIView.setViewport(sf::FloatRect({0.0f, 0.0f}, 
        {(dividerPosition - dividerWidth / (2.0f * windowSize.x)), 1.0f}));
    window.setView(leftUIView);
    leftView.render(target);
    
    // 渲染右侧视图的滚动条
    sf::View rightUIView = rightView.getUIView();
    float rightViewportLeft = dividerPosition + dividerWidth / (2.0f * windowSize.x);
    float rightViewportWidth = (1.0f - dividerPosition) - dividerWidth / (2.0f * windowSize.x);
    rightUIView.setViewport(sf::FloatRect({rightViewportLeft, 0.0f}, {rightViewportWidth, 1.0f}));
    window.setView(rightUIView);
    rightView.render(target);
    
    // 恢复默认视图
    window.setView(window.getDefaultView());
}