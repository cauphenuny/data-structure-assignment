#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>

class TreeRenderer {
public:
    // 树节点结构
    struct TreeNode {
        sf::Vector2f position;
        int value;
        int id;
        inline static size_t nodeCount;
        
        TreeNode(sf::Vector2f pos, int val);
    };
    
    // 边结构
    struct TreeEdge {
        int from;
        int to;
        
        TreeEdge(int from, int to);
    };
    
    TreeRenderer(const sf::Font& font, float nodeRadius = 25.0f);
    
    int addNode(sf::Vector2f position, int value);
    void addEdge(int fromNodeId, int toNodeId);
    
    void clear();
    void render(sf::RenderTarget& target);
    void update(float deltaTime);
    
    // 获取树的边界框
    sf::FloatRect getBounds() const;
    
private:
    std::vector<TreeNode> nodes;
    std::vector<TreeEdge> edges;
    std::vector<sf::CircleShape> nodeShapes;
    std::vector<sf::Text> nodeLabels;
    
    const sf::Font& font;
    float nodeRadius;
    sf::Color nodeOutlineColor;
    sf::Color textColor;
    sf::Color lineColor;
    
    sf::Clock clock;

    void updateVisuals();
    void drawFeatheredLine(sf::RenderTarget& target, const sf::Vector2f& start, 
                         const sf::Vector2f& end, sf::Color color, 
                         float thickness = 1.0f, float feather = 1.0f);
};