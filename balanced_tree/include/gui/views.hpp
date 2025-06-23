#pragma once
#include "gui/scroll_manager.hpp"

#include <SFML/Graphics.hpp>

class ScrollManager;

class View {
public:
    View(sf::RenderWindow& window,
        sf::Vector2f initialSize, const sf::Vector2f& windowSize,
        const sf::Vector2f& viewPosition, float minZoom = 0.1f, float maxZoom = 5.0f);
    
    const sf::View& getContentView() const;
    const sf::View& getUIView() const;
    
    // 视图变换
    virtual void setZoom(float factor, sf::Vector2i focusPoint);
    virtual void pan(sf::Vector2f delta);
    virtual void scroll(sf::Vector2f delta, DragMode dragmode);
    
    void fitContent(sf::FloatRect contentBounds, float padding = 50.0f);
    
    sf::Vector2f& getViewOffset();
    sf::Vector2f getMaxOffset();
    sf::Vector2f getWindowSize() const;
    sf::Vector2f getViewPosition() const;
    float getEffectiveZoom();
    
    DragMode getMouseDragMode(sf::Vector2f mousePos);

    void setViewPosition(const sf::Vector2f& position);
    
    void updateViews();
    void updateUIView();
    void updateContentView();
    
    void updateScrollbars();

    void render(sf::RenderTarget& target);
    
protected:
    sf::View contentView;
    sf::View uiView;

    sf::RenderWindow& window;
    const sf::Vector2f& viewSize;
    const sf::Vector2f& viewPosition;
    
    sf::Vector2f initialSize;
    sf::Vector2f viewOffset;

    ScrollManager scrollManager;

    float zoom;
    const float minZoom;
    const float maxZoom;
    float effectiveZoom;

    friend class ScrollManager;
};

class CanvasView : public View {
public:
    CanvasView(sf::RenderWindow& window,
        sf::Vector2f initialSize, const sf::Vector2f& windowSize,
        const sf::Vector2f& viewPosition, float minZoom = 0.1f, float maxZoom = 5.0f);
    void setZoom(float factor, sf::Vector2i focusPoint) override;
    void pan(sf::Vector2f delta) override;
    // void scroll(sf::Vector2f delta, DragMode dragmode) override;
};