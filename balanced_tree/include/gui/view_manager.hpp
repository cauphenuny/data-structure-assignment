#pragma once
#include <SFML/Graphics.hpp>

class ViewManager {
public:
    ViewManager(const sf::RenderWindow& window,
        sf::Vector2f initialSize, const sf::Vector2f& windowSize,
        float minZoom = 0.1f, float maxZoom = 5.0f);
    
    const sf::View& getContentView() const;
    const sf::View& getUIView() const;
    
    // 视图变换
    void setZoom(float factor, sf::Vector2i focusPoint);
    void pan(sf::Vector2f delta);
    
    void fitContent(sf::FloatRect contentBounds, float padding = 50.0f);
    
    sf::Vector2f getViewOffset() const;
    sf::Vector2f getWindowSize() const;
    float getEffectiveZoom();
    
    void update();
    void updateUIView();
    void updateContentView();
    
private:
    sf::View contentView;
    sf::View uiView;

    const sf::RenderWindow& window;
    const sf::Vector2f& windowSize;
    
    sf::Vector2f initialSize;
    sf::Vector2f viewOffset;
    float zoom;
    const float minZoom;
    const float maxZoom;
    float effectiveZoom;
};