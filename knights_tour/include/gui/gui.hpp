/// @file gui.hpp
/// @brief GUI functions

#pragma once
#include "gui/events.hpp"
#include "gui/chessboard_renderer.hpp"
#include "gui/control_panel.hpp"
#include "gui/split_view.hpp"
#include "algorithm.h"

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

class KnightsTourGUI : public EventTarget {
public:
    // 构造函数
    KnightsTourGUI();
    
    // 主循环方法
    void run();
    
protected:
    // 窗口和视图
    sf::RenderWindow window;
    sf::ContextSettings settings;
    
    // 状态变量
    sf::Vector2f lastMousePos;
    
    // 配置常量
    const unsigned int initialWidth;
    const unsigned int initialHeight;
    sf::Vector2f windowSize;
    
    // 资源
    sf::Font font;
    
    ChessboardRenderer chessboardRenderer;
    ControlPanel controlPanel;
    SplitView splitView;
    
    // 颜色配置
    sf::Color backgroundColor;
    
    // 初始化方法
    void initWindow();
    void initEventListeners();
    
    void update(float deltaTime);
    void render();
};

void gui();
