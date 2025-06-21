/// @file gui.h
/// @brief GUI functions

#pragma once
#include "gui/events.hpp"
#include "gui/tree_renderer.hpp"
#include "gui/view_manager.hpp"
#include "tree/interface.hpp"

#include <SFML/Graphics.hpp>

#include <cstdint>
#include <vector>

/****************************** Definition ********************************/

enum class LogLevel : uint8_t {
    INFO,
    WARNING,
    ERROR,
    DEBUG,
};

// 拖动模式枚举
enum class DragMode : uint8_t {
    NONE,
    VIEW_DRAG,
    HSCROLL_DRAG,
    VSCROLL_DRAG
};

class GUIBase : public EventTarget {
public:
    // 构造函数
    GUIBase();
    
    // 主循环方法
    void run();
    
private:
    // 窗口和视图
    sf::RenderWindow window;

    sf::ContextSettings settings;

    sf::Text titleText;
    
    // UI元素
    sf::RectangleShape horizontalScrollbar;
    sf::RectangleShape horizontalScrollbarHandle;
    sf::RectangleShape verticalScrollbar;
    sf::RectangleShape verticalScrollbarHandle;
    
    // 状态变量
    DragMode dragMode;
    sf::Vector2f lastMousePos;
    
    // 配置常量
    const unsigned int initialWidth;
    const unsigned int initialHeight;
    const float scrollbarSize;
    sf::Vector2f windowSize;
    
    TreeRenderer treeRenderer;
    ViewManager viewManager;
    
    // 颜色配置
    sf::Color backgroundColor;
    sf::Color lineColor;
    sf::Color nodeOutlineColor;
    sf::Color textColor;
    sf::Color scrollbarColor;
    sf::Color scrollbarHandleColor;
    
    // 资源
    sf::Font font;
    sf::Clock clock;
    
    // 初始化方法
    void initWindow();
    void initTreeData();
    void initUI();
    
    // 更新方法
    void updateScrollbars();

    // 事件处理方法
    void initEventListeners();
    
    // 渲染方法
    void render();
    
    // // 让事件处理器类可以访问私有成员
    // friend class EventHandlerBase;
    // friend class WindowResizeHandler;
    // friend class MouseWheelHandler;
    // friend class MousePressHandler;
    // friend class MouseReleaseHandler;
    // friend class MouseMoveHandler;
};

void echo(std::string_view message, LogLevel level = LogLevel::INFO);

void gui();

void drawTree(TreeBase* tree);

// NOTE: don't forget to implement Tree<Key, Value>::print()
