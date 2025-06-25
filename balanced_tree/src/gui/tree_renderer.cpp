#include "gui/tree_renderer.hpp"
#include <cmath>

// size_t TreeRenderer::TreeNode::nodeCount = 0;

TreeRenderer::TreeNode::TreeNode(sf::Vector2f pos, int val)
    : position(pos), value(val), id(nodeCount++) {}

TreeRenderer::TreeEdge::TreeEdge(int from, int to)
    : from(from), to(to) {}

TreeRenderer::TreeRenderer(const sf::Font& font, float nodeRadius)
    : font(font), 
      nodeRadius(nodeRadius),
      nodeOutlineColor(10, 136, 204),
      textColor(30, 30, 40),
      lineColor(10, 136, 204) {
    edges.reserve(20);
}

int TreeRenderer::addNode(sf::Vector2f position, int value) {
    nodes.emplace_back(position - sf::Vector2f{nodeRadius, nodeRadius}, value);
    updateVisuals();
    return nodes.back().id;
}

void TreeRenderer::addEdge(int fromNodeId, int toNodeId) {
    // 确保索引有效
    if (fromNodeId >= 0 && fromNodeId < nodes.size() && 
        toNodeId >= 0 && toNodeId < nodes.size()) {
        edges.emplace_back(fromNodeId, toNodeId);
    }
}

void TreeRenderer::clear() {
    nodes.clear();
    edges.clear();
    nodeShapes.clear();
    nodeLabels.clear();
}

void TreeRenderer::updateVisuals() {
    nodeShapes.clear();
    nodeLabels.clear();
    
    for (const auto& node : nodes) {
        // 创建节点形状
        sf::CircleShape circle(nodeRadius);
        circle.setPointCount(120);
        circle.setFillColor(sf::Color::White);
        circle.setOutlineColor(nodeOutlineColor);
        circle.setOutlineThickness(2);
        circle.setPosition(node.position - sf::Vector2f(nodeRadius, nodeRadius));
        nodeShapes.push_back(circle);
        
        // 创建节点标签
        sf::Text label(font, std::to_string(node.value), 20);
        label.setFillColor(textColor);
        
        // 居中标签
        sf::FloatRect bounds = label.getLocalBounds();
        label.setOrigin(bounds.size / 2.0f);
        label.setPosition(node.position);
        
        nodeLabels.push_back(label);
    }
}

void TreeRenderer::render(sf::RenderTarget& target) {
    // 绘制边
    for (const auto& edge : edges) {
        sf::Vector2f startPos = nodes[edge.from].position;
        sf::Vector2f endPos = nodes[edge.to].position;
        
        drawFeatheredLine(target, startPos, endPos, lineColor, 1.5f, 1.0f);
    }
    
    // 绘制节点和标签
    for (const auto& shape : nodeShapes) {
        target.draw(shape);
    }
    
    for (const auto& label : nodeLabels) {
        target.draw(label);
    }
}

void TreeRenderer::update(float deltaTime) {
    // 添加节点动画效果
    float time = clock.getElapsedTime().asSeconds();
    for (size_t i = 0; i < nodeShapes.size(); i++) {
        int r = 100 + static_cast<int>(50 * std::sin(time + i));
        int g = 150 + static_cast<int>(50 * std::sin(time * 0.7f + i));
        int b = 250 + static_cast<int>(50 * std::sin(time * 0.5f + i));
        nodeShapes[i].setFillColor(sf::Color(r, g, b));
    }
}

sf::FloatRect TreeRenderer::getBounds() const {
    if (nodes.empty()) return sf::FloatRect({0, 0}, {0, 0});
    
    float minX = nodes[0].position.x;
    float minY = nodes[0].position.y;
    float maxX = nodes[0].position.x;
    float maxY = nodes[0].position.y;
    
    for (const auto& node : nodes) {
        minX = std::min(minX, node.position.x - nodeRadius);
        minY = std::min(minY, node.position.y - nodeRadius);
        maxX = std::max(maxX, node.position.x + nodeRadius);
        maxY = std::max(maxY, node.position.y + nodeRadius);
    }
    
    return sf::FloatRect({minX, minY}, {maxX - minX, maxY - minY});
}

void TreeRenderer::drawFeatheredLine(sf::RenderTarget& target, const sf::Vector2f& start, 
                                    const sf::Vector2f& end, sf::Color color, 
                                    float thickness, float feather) {
    sf::Vector2f dir = end - start;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len == 0) return;
    dir /= len;
    sf::Vector2f normal(-dir.y, dir.x);

    thickness /= 2.f;
    sf::Vector2f half_n  = normal * thickness;
    sf::Vector2f outer_n = normal * (thickness + feather);

    sf::Color transparent = color;
    transparent.a = 0;

    sf::VertexArray triangles(sf::PrimitiveType::TriangleStrip, 8);

    triangles[0] = {start + outer_n,  transparent};
    triangles[1] = { end  + outer_n,  transparent};
    triangles[2] = {start + half_n,   color      };
    triangles[3] = { end  + half_n,   color      };
    triangles[4] = {start - half_n,   color      };
    triangles[5] = { end  - half_n,   color      };
    triangles[6] = {start - outer_n,  transparent};
    triangles[7] = { end  - outer_n,  transparent};

    target.draw(triangles);
}