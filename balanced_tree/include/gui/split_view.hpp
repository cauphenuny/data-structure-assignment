#pragma once
#include "gui/views.hpp"
#include <SFML/Graphics.hpp>
#include <functional>

class SplitView {
public:
    using RenderCallback = std::function<void(sf::RenderWindow&, const sf::View&)>;

    SplitView(sf::RenderWindow& window, const sf::Vector2f& windowSize,
        sf::Vector2f& lastMousePos);
    
    // 初始化左右视图
    void initialize(sf::Vector2f leftContentSize, sf::Vector2f rightContentSize);
    
    // 更新左右视图大小
    void updateViews();

    // 更新背景
    void updateBackgrounds();
    
    void render(sf::RenderTarget& target);
    void renderScrollbars(sf::RenderTarget& target);
    
    // 事件处理
    void handleMouseMove(sf::Vector2i mousePos, DragMode dragMode);
    DragMode handleMousePress(sf::Vector2i mousePos);
    void handleMouseRelease();
    void handleMouseWheel(sf::Vector2i mousePos, float delta);

    void setLeftViewRenderer(RenderCallback callback) { leftViewRenderer = callback; }
    void setRightViewRenderer(RenderCallback callback) { rightViewRenderer = callback; }
    
    // 获取各视图
    CanvasView& getLeftView() { return leftView; }
    View& getRightView() { return rightView; }
    
private:
    sf::RenderWindow& window;
    sf::Vector2f windowSize;
    
    // 分隔线
    sf::RectangleShape divider;
    float dividerPosition;  // 0.0-1.0之间的比例
    float dividerWidth;
    bool isDraggingDivider;

    sf::Vector2f leftViewSize;
    sf::Vector2f rightViewSize;
    sf::Vector2f leftViewPosition;
    sf::Vector2f rightViewPosition;
    
    CanvasView leftView;
    View rightView;

    sf::Vector2f& lastMousePos;

    RenderCallback leftViewRenderer;
    RenderCallback rightViewRenderer;

    sf::RectangleShape leftBackground;
    sf::RectangleShape rightBackground;
    
    // 判断鼠标是否在分隔线上
    bool isMouseOverDivider(sf::Vector2i mousePos);
    
    // 更新分隔线位置
    void updateDividerPosition(float newPosition);
    void setCursorType(sf::Cursor::Type type);
};