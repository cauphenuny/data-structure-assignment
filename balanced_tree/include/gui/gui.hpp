/// @file gui.h
/// @brief GUI functions

#pragma once
#include "gui/events.hpp"
#include "tree/interface.hpp"

#include <SFML/Graphics.hpp>

#include <cstdint>
#include <string_view>
#include <vector>
#include <optional>
#include <functional>
#include <map>

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
    // 树节点结构
    struct TreeNode {
        inline static size_t nodeCount = 0;
        sf::Vector2f position;
        int value;
        int id;

        TreeNode(sf::Vector2f pos, int val)
            : position(pos), value(val), id(nodeCount) {
                nodeCount++;
            }
    };
    
    // 边结构
    struct TreeEdge {
        int from;
        int to;

        TreeEdge(int _from, int _to)
            : from(_from), to(_to) {}
    };
    
    // 窗口和视图
    sf::RenderWindow window;
    sf::View contentView;
    sf::View uiView;

    sf::ContextSettings settings;
    
    // 树状结构数据
    std::vector<TreeNode> treeNodes;
    std::vector<TreeEdge> treeEdges;

    // 绘制元素
    std::vector<sf::CircleShape> nodes;
    std::vector<sf::Text> nodeLabels;
    std::vector<sf::Vertex> edges;

    sf::Text titleText;
    
    // UI元素
    sf::RectangleShape horizontalScrollbar;
    sf::RectangleShape horizontalScrollbarHandle;
    sf::RectangleShape verticalScrollbar;
    sf::RectangleShape verticalScrollbarHandle;
    
    // 状态变量
    DragMode dragMode;
    sf::Vector2f lastMousePos;
    sf::Vector2f viewOffset;
    float zoom;
    const float minZoom;
    const float maxZoom;
    
    // 配置常量
    const unsigned int initialWidth;
    const unsigned int initialHeight;
    const float scrollbarSize;
    const float nodeRadius;
    sf::Vector2f windowSize;
    
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
    void updateAnimation(float deltaTime);
    inline void updateContentView();
    inline void updateContentView(float effectiveZoom);

    // 事件处理方法
    void initEventListeners();
    
    // 渲染方法
    void render();
    void drawNodes();
    void drawEdges();
    void drawUI();
    
    // 辅助方法
    inline float getEffectiveZoom();
    void drawFeatheredLine(const sf::Vector2f& start, const sf::Vector2f& end,
                          sf::Color color, float thickness, float feather);
    
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
