#include "gui/balanced_tree_renderer.hpp"
#include <cmath>
#include <algorithm>
#include <queue>
#include <unordered_set>
#include <limits>

BalancedTreeRenderer::BalancedTreeRenderer(const sf::Font& font, float nodeRadius)
    : font(font), 
      nodeRadius(nodeRadius),
      horizontalSpacing(80.0f),
      verticalSpacing(80.0f),
      animationSpeed(2.0f),
      nodeOutlineColor(10, 136, 204),
      textColor(30, 30, 40),
      lineColor(10, 136, 204),
      highlightColor(255, 100, 100),
      fadeColor(200, 200, 200) {
}

void BalancedTreeRenderer::updateFromTree(TreeBase* tree) {
    if (!tree) {
        clear();
        return;
    }
    
    clear();
    
    auto forestView = tree->view();
    
    for (const auto& root : forestView) {
        if (root) {
            buildRenderData(root.get());
        }
    }
    
    calculateLayout();
    updateVisuals();
}

void BalancedTreeRenderer::buildRenderData(NodeView* node) {
    if (!node) return;
    
    auto [key, value] = node->content();
    
    nodes.emplace(node->id(), RenderNode({0, 0}, key, value, node->id()));
    
    if (node->child[0]) {
        edges.emplace_back(node->id(), node->child[0]->id());
        buildRenderData(node->child[0].get());
    }
    
    if (node->child[1]) {
        edges.emplace_back(node->id(), node->child[1]->id());
        buildRenderData(node->child[1].get());
    }
}

void BalancedTreeRenderer::calculateLayout() {
    if (nodes.empty()) return;
    
    std::vector<const void*> roots;
    for (const auto& [id, node] : nodes) {
        bool isRoot = true;
        for (const auto& edge : edges) {
            if (edge.toId == id) {
                isRoot = false;
                break;
            }
        }
        if (isRoot) {
            roots.push_back(id);
        }
    }
    
    float currentX = 100.0f;
    for (const void* rootId : roots) {
        calculateSubtreeLayout(rootId, currentX, 80.0f);
        currentX += getSubtreeWidth(rootId) + horizontalSpacing;
    }
}

float BalancedTreeRenderer::getSubtreeWidth(const void* nodeId) const {
    std::vector<const void*> children;
    for (const auto& edge : edges) {
        if (edge.fromId == nodeId) {
            children.push_back(edge.toId);
        }
    }
    
    if (children.empty()) {
        return nodeRadius * 2.5f;
    }
    
    float totalWidth = 0;
    for (const void* child : children) {
        totalWidth += getSubtreeWidth(child);
    }
    
    return std::max(totalWidth, nodeRadius * 2.5f);
}

void BalancedTreeRenderer::calculateSubtreeLayout(const void* nodeId, float x, float y) {
    auto it = nodes.find(nodeId);
    if (it == nodes.end()) return;
    
    std::vector<const void*> children;
    for (const auto& edge : edges) {
        if (edge.fromId == nodeId) {
            children.push_back(edge.toId);
        }
    }
    
    if (children.empty()) {
        it->second.targetPosition = {x, y};
        return;
    }
    
    std::vector<float> childWidths;
    float totalChildWidth = 0;
    for (const void* child : children) {
        float width = getSubtreeWidth(child);
        childWidths.push_back(width);
        totalChildWidth += width;
    }
    
    it->second.targetPosition = {x + totalChildWidth / 2, y};
    
    float currentChildX = x;
    for (size_t i = 0; i < children.size(); ++i) {
        calculateSubtreeLayout(children[i], currentChildX + childWidths[i] / 2, y + verticalSpacing);
        currentChildX += childWidths[i];
    }
}

void BalancedTreeRenderer::animateTransition(const ForestView& fromView, const ForestView& toView) {
    // 收集旧视图和新视图的节点ID
    auto oldNodes = collectNodeIds(fromView);
    auto newNodes = collectNodeIds(toView);
    
    // 找出新增、删除和保留的节点
    std::unordered_set<const void*> addedNodes;
    std::unordered_set<const void*> removedNodes;
    std::unordered_set<const void*> commonNodes;
    
    for (const void* id : newNodes) {
        if (oldNodes.find(id) == oldNodes.end()) {
            addedNodes.insert(id);
        } else {
            commonNodes.insert(id);
        }
    }
    
    for (const void* id : oldNodes) {
        if (newNodes.find(id) == newNodes.end()) {
            removedNodes.insert(id);
        }
    }
    
    // 首先更新到新的树结构
    clear();
    for (const auto& root : toView) {
        if (root) {
            buildRenderData(root.get());
        }
    }
    calculateLayout();
    
    // 设置动画
    for (auto& [id, node] : nodes) {
        if (addedNodes.find(id) != addedNodes.end()) {
            // 新节点：从透明逐渐出现
            node.animationType = NodeAnimationType::FADE_IN;
            node.animationProgress = 0.0f;
            node.alpha = 0.0f;
            node.scale = 0.5f;
            node.currentColor = node.baseColor;
        } else if (commonNodes.find(id) != commonNodes.end()) {
            // 保留节点：可能需要移动
            // 这里应该从之前的位置开始动画，但为了简化，我们使用当前逻辑
            node.animationType = NodeAnimationType::MOVE;
            node.animationProgress = 0.0f;
            node.startPosition = node.position;
            // targetPosition 已经在 calculateLayout 中设置
        }
    }
    
    // 处理边的动画
    for (size_t i = 0; i < edges.size(); ++i) {
        edges[i].animationType = EdgeAnimationType::FADE_IN;
        edges[i].animationProgress = 0.0f;
        edges[i].alpha = 0.0f;
    }
    
    updateVisuals();
}

std::unordered_set<const void*> BalancedTreeRenderer::collectNodeIds(const ForestView& view) const {
    std::unordered_set<const void*> ids;
    
    std::function<void(NodeView*)> collect = [&](NodeView* node) {
        if (!node) return;
        ids.insert(node->id());
        collect(node->child[0].get());
        collect(node->child[1].get());
    };
    
    for (const auto& root : view) {
        collect(root.get());
    }
    
    return ids;
}

void BalancedTreeRenderer::updateFromTrace(const std::vector<ForestView>& trace, size_t currentStep) {
    if (currentStep >= trace.size()) return;
    
    if (currentStep == 0) {
        // 第一步，直接显示
        clear();
        const auto& currentForest = trace[currentStep];
        for (const auto& root : currentForest) {
            if (root) {
                buildRenderData(root.get());
            }
        }
        calculateLayout();
        updateVisuals();
    } else {
        // 使用动画过渡
        animateTransition(trace[currentStep - 1], trace[currentStep]);
    }
}

void BalancedTreeRenderer::updateNodeAnimations(float deltaTime) {
    for (auto& [id, node] : nodes) {
        if (!node.isAnimating()) continue;
        
        node.animationProgress += deltaTime * animationSpeed;
        if (node.animationProgress >= 1.0f) {
            node.animationProgress = 1.0f;
        }
        
        float t = easeInOutCubic(node.animationProgress);
        
        switch (node.animationType) {
            case NodeAnimationType::FADE_IN:
                node.alpha = 255.0f * t;
                node.scale = 0.5f + 0.5f * t;
                break;
                
            case NodeAnimationType::FADE_OUT:
                node.alpha = 255.0f * (1.0f - t);
                node.scale = 1.0f - 0.5f * t;
                break;
                
            case NodeAnimationType::MOVE:
                node.position = lerp(node.startPosition, node.targetPosition, t);
                break;
                
            case NodeAnimationType::HIGHLIGHT:
                {
                    float intensity = std::sin(t * 3.14159f);
                    node.currentColor = sf::Color(
                        static_cast<uint8_t>(node.baseColor.r + (highlightColor.r - node.baseColor.r) * intensity),
                        static_cast<uint8_t>(node.baseColor.g + (highlightColor.g - node.baseColor.g) * intensity),
                        static_cast<uint8_t>(node.baseColor.b + (highlightColor.b - node.baseColor.b) * intensity)
                    );
                }
                break;
                
            default:
                break;
        }
        
        if (node.animationProgress >= 1.0f) {
            // 动画完成
            switch (node.animationType) {
                case NodeAnimationType::FADE_IN:
                    node.alpha = 255.0f;
                    node.scale = 1.0f;
                    break;
                case NodeAnimationType::MOVE:
                    node.position = node.targetPosition;
                    break;
                default:
                    break;
            }
            node.animationType = NodeAnimationType::NONE;
        }
    }
}

void BalancedTreeRenderer::updateEdgeAnimations(float deltaTime) {
    for (auto& edge : edges) {
        if (!edge.isAnimating()) continue;
        
        edge.animationProgress += deltaTime * animationSpeed;
        if (edge.animationProgress >= 1.0f) {
            edge.animationProgress = 1.0f;
        }
        
        float t = easeInOutCubic(edge.animationProgress);
        
        switch (edge.animationType) {
            case EdgeAnimationType::FADE_IN:
                edge.alpha = 255.0f * t;
                break;
                
            case EdgeAnimationType::FADE_OUT:
                edge.alpha = 255.0f * (1.0f - t);
                break;
                
            default:
                break;
        }
        
        if (edge.animationProgress >= 1.0f) {
            edge.animationType = EdgeAnimationType::NONE;
            if (edge.animationType == EdgeAnimationType::FADE_IN) {
                edge.alpha = 255.0f;
            }
        }
    }
}

bool BalancedTreeRenderer::hasActiveAnimations() const {
    for (const auto& [id, node] : nodes) {
        if (node.isAnimating()) return true;
    }
    
    for (const auto& edge : edges) {
        if (edge.isAnimating()) return true;
    }
    
    return false;
}

void BalancedTreeRenderer::clear() {
    nodes.clear();
    edges.clear();
    nodeShapes.clear();
    nodeLabels.clear();
}

void BalancedTreeRenderer::update(float deltaTime) {
    updateNodeAnimations(deltaTime);
    updateEdgeAnimations(deltaTime);
    updateVisuals();
}

void BalancedTreeRenderer::updateVisuals() {
    nodeShapes.clear();
    nodeLabels.clear();
    
    for (const auto& [id, node] : nodes) {
        sf::CircleShape circle(nodeRadius * node.scale);
        circle.setPointCount(32);
        
        sf::Color color = node.currentColor;
        color.a = static_cast<uint8_t>(node.alpha);
        circle.setFillColor(color);
        circle.setOutlineColor(nodeOutlineColor);
        circle.setOutlineThickness(2);
        circle.setPosition(node.position - sf::Vector2f(nodeRadius * node.scale, nodeRadius * node.scale));
        nodeShapes.push_back(circle);
        
        std::string labelText = node.key;
        if (!node.value.empty() && node.value != node.key) {
            labelText += ":" + node.value;
        }
        
        sf::Text label(font, labelText, static_cast<unsigned int>(16 * node.scale));
        sf::Color textCol = textColor;
        textCol.a = static_cast<uint8_t>(node.alpha);
        label.setFillColor(textCol);
        
        sf::FloatRect bounds = label.getLocalBounds();
        label.setOrigin(bounds.size / 2.0f);
        label.setPosition(node.position);
        
        nodeLabels.push_back(label);
    }
}

void BalancedTreeRenderer::render(sf::RenderTarget& target) {
    // 绘制边
    for (const auto& edge : edges) {
        auto fromIt = nodes.find(edge.fromId);
        auto toIt = nodes.find(edge.toId);
        
        if (fromIt != nodes.end() && toIt != nodes.end()) {
            sf::Color edgeColor = lineColor;
            edgeColor.a = static_cast<uint8_t>(edge.alpha);
            
            drawFeatheredLine(target, fromIt->second.position, toIt->second.position, 
                            edgeColor, 1.5f, 1.0f);
        }
    }
    
    // 绘制节点和标签
    for (const auto& shape : nodeShapes) {
        target.draw(shape);
    }
    
    for (const auto& label : nodeLabels) {
        target.draw(label);
    }
}

sf::FloatRect BalancedTreeRenderer::getBounds() const {
    if (nodes.empty()) return sf::FloatRect({0, 0}, {0, 0});
    
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();
    
    for (const auto& [id, node] : nodes) {
        minX = std::min(minX, node.position.x - nodeRadius);
        minY = std::min(minY, node.position.y - nodeRadius);
        maxX = std::max(maxX, node.position.x + nodeRadius);
        maxY = std::max(maxY, node.position.y + nodeRadius);
    }
    
    return sf::FloatRect({minX, minY}, {maxX - minX, maxY - minY});
}

void BalancedTreeRenderer::drawFeatheredLine(sf::RenderTarget& target, const sf::Vector2f& start, 
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

sf::Vector2f BalancedTreeRenderer::lerp(const sf::Vector2f& a, const sf::Vector2f& b, float t) const {
    return a + (b - a) * t;
}

float BalancedTreeRenderer::easeInOutCubic(float t) const {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}