#pragma once
#include "tree/interface.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>

enum class NodeAnimationType {
    NONE,
    FADE_IN,      // 新节点出现
    FADE_OUT,     // 节点消失
    MOVE,         // 节点移动（旋转等操作）
    HIGHLIGHT,    // 节点高亮（查找等操作）
    ROTATING      // 旋转动画
};

enum class EdgeAnimationType {
    NONE,
    FADE_IN,      // 新边出现
    FADE_OUT,     // 边消失
    RECONNECT     // 边重连（旋转时）
};

class BalancedTreeRenderer {
public:
    // 渲染节点信息
    struct RenderNode {
        sf::Vector2f position;
        sf::Vector2f startPosition;  // 动画起始位置
        sf::Vector2f targetPosition; // 动画目标位置
        std::string key;
        std::string value;
        const void* id; // NodeView的唯一标识
        
        // 动画状态
        NodeAnimationType animationType = NodeAnimationType::NONE;
        float animationProgress = 0.0f; // 0.0 到 1.0
        float animationDuration = 1.0f;
        sf::Color baseColor = sf::Color::White;
        sf::Color currentColor = sf::Color::White;
        float alpha = 255.0f;
        float scale = 1.0f;
        
        RenderNode(sf::Vector2f pos, const std::string& k, const std::string& v, const void* nodeId)
            : position(pos), startPosition(pos), targetPosition(pos), key(k), value(v), id(nodeId) {}
            
        bool isAnimating() const { return animationType != NodeAnimationType::NONE && animationProgress < 1.0f; }
    };
    
    // 渲染边信息
    struct RenderEdge {
        const void* fromId;
        const void* toId;
        
        // 动画状态
        EdgeAnimationType animationType = EdgeAnimationType::NONE;
        float animationProgress = 0.0f;
        float animationDuration = 1.0f;
        float alpha = 255.0f;
        
        RenderEdge(const void* from, const void* to) : fromId(from), toId(to) {}
        
        bool isAnimating() const { return animationType != EdgeAnimationType::NONE && animationProgress < 1.0f; }
    };
    
    BalancedTreeRenderer(const sf::Font& font, float nodeRadius = 30.0f);
    
    // 从TreeBase更新渲染数据
    void updateFromTree(TreeBase* tree);
    
    // 动画支持 - 比较两个ForestView并生成动画
    void animateTransition(const ForestView& fromView, const ForestView& toView);
    
    // 从操作trace更新（带动画）
    void updateFromTrace(const std::vector<ForestView>& trace, size_t currentStep);
    
    void clear();
    void render(sf::RenderTarget& target);
    void update(float deltaTime);
    
    // 获取树的边界框
    sf::FloatRect getBounds() const;
    
    // 设置动画参数
    void setAnimationSpeed(float speed) { animationSpeed = speed; }
    void setNodeSpacing(float horizontal, float vertical) { 
        horizontalSpacing = horizontal; 
        verticalSpacing = vertical; 
    }
    
    // 检查是否有动画正在进行
    bool hasActiveAnimations() const;
    
private:
    std::unordered_map<const void*, RenderNode> nodes;
    std::vector<RenderEdge> edges;
    std::vector<sf::CircleShape> nodeShapes;
    std::vector<sf::Text> nodeLabels;
    
    const sf::Font& font;
    float nodeRadius;
    float horizontalSpacing;
    float verticalSpacing;
    float animationSpeed;
    
    sf::Color nodeOutlineColor;
    sf::Color textColor;
    sf::Color lineColor;
    sf::Color highlightColor;
    sf::Color fadeColor;
    
    // 布局计算
    void calculateLayout();
    float getSubtreeWidth(const void* nodeId) const;
    void calculateSubtreeLayout(const void* nodeId, float x, float y);
    
    // 动画系统
    void updateNodeAnimations(float deltaTime);
    void updateEdgeAnimations(float deltaTime);
    void startNodeAnimation(const void* nodeId, NodeAnimationType type, float duration = 1.0f);
    void startEdgeAnimation(size_t edgeIndex, EdgeAnimationType type, float duration = 1.0f);
    
    // 动画比较和生成
    void compareAndAnimate(const ForestView& oldView, const ForestView& newView);
    std::unordered_set<const void*> collectNodeIds(const ForestView& view) const;
    
    // 渲染辅助
    void updateVisuals();
    void drawFeatheredLine(sf::RenderTarget& target, const sf::Vector2f& start, 
                         const sf::Vector2f& end, sf::Color color, 
                         float thickness = 1.0f, float feather = 1.0f);
    
    // 递归遍历NodeView构建渲染数据
    void buildRenderData(NodeView* node);
    
    // 工具函数
    sf::Vector2f lerp(const sf::Vector2f& a, const sf::Vector2f& b, float t) const;
    float easeInOutCubic(float t) const;
};