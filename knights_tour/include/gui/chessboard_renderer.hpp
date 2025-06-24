#pragma once

#include <SFML/Graphics.hpp>
#include "board.h"
#include "algorithm.h"
#include <vector>
#include <string>

class ChessboardRenderer {
public:
    ChessboardRenderer(sf::Font& font);
    
    void render(sf::RenderWindow& window);
    void update(float deltaTime);
    
    // 设置起始位置
    void setStartPosition(int x, int y);
    
    // 运行算法
    void runAlgorithm(Algorithm algo);
    
    // 暂停/继续动画
    void togglePause();
    
    // 控制速度
    void setSpeed(float speed);
    
    // 重置棋盘
    void reset();
    
    // 跳到下一步
    void nextStep();
    
    // 跳到上一步
    void prevStep();
    
    // 获取当前状态
    bool isRunning() const { return running; }
    bool isPaused() const { return paused; }
    bool isCompleted() const { return completed; }
    
private:
    void drawBoard(sf::RenderWindow& window);
    void drawKnight(sf::RenderWindow& window);
    void drawNumbers(sf::RenderWindow& window);
    void drawPath(sf::RenderWindow& window);
    void processNextStep();
    
    const float CELL_SIZE = 60.0f;
    const float BOARD_OFFSET_X = 50.0f;
    const float BOARD_OFFSET_Y = 50.0f;
    
    sf::Font& font;
    sf::Texture knightTexture;
    sf::Sprite knight;
    
    // 棋盘状态
    int boardState[8][8];
    Point knightPosition;
    Point startPosition;
    
    // 算法相关
    std::vector<Path> solutions;
    int currentSolution;
    int currentStep;
    
    // 动画相关
    bool running;
    bool paused;
    bool completed;
    float animationSpeed;
    float animationTime;
    float stepDuration;
    
    // 当前动画的起点和终点
    Point animStartPos;
    Point animEndPos;
    float animProgress;
    
    // 状态消息
    std::string statusMessage;
};