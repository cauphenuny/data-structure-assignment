/// @file gui.h
/// @brief GUI functions

#pragma once
#include "gui/events.hpp"
#include "gui/tree_renderer.hpp"
#include "gui/views.hpp"
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
    
    // 状态变量
    DragMode dragMode;
    sf::Vector2f lastMousePos;
    
    // 配置常量
    const unsigned int initialWidth;
    const unsigned int initialHeight;
    const float scrollbarSize;
    sf::Vector2f windowSize;
    
    TreeRenderer treeRenderer;
    CanvasView canvasView;
    
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

    void initEventListeners();
    
    void render();
};

void echo(std::string_view message, LogLevel level = LogLevel::INFO);

void gui();

void drawTree(TreeBase* tree);

// NOTE: don't forget to implement Tree<Key, Value>::print()
