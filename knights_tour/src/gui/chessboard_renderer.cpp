#include "gui/chessboard_renderer.hpp"
#include <iostream>
#include <cmath>

ChessboardRenderer::ChessboardRenderer(sf::Font& font)
    : font(font),
      running(false),
      paused(false),
      completed(false),
      animationSpeed(1.0f),
      animationTime(0.0f),
      stepDuration(1.0f),
      currentSolution(0),
      currentStep(0),
      animProgress(0.0f),
      knight(knightTexture) {
    
    // 初始化棋盘状态
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            boardState[i][j] = 0;
        }
    }
    
    // 加载马的纹理
    if (!knightTexture.loadFromFile("assets/knight.png") &&
        !knightTexture.loadFromFile("../assets/knight.png") &&
        !knightTexture.loadFromFile("../../assets/knight.png")) {
        std::cerr << "Failed to load knight texture." << std::endl;
        // 创建一个备用纹理
        knightTexture.resize({48, 48});
    }
    
    knight.setTexture(knightTexture);
    knight.setScale({CELL_SIZE / knightTexture.getSize().x * 0.8f, 
                   CELL_SIZE / knightTexture.getSize().y * 0.8f});
    knight.setOrigin(static_cast<sf::Vector2f>(knightTexture.getSize()) / 2.0f);
    
    // 设置默认起始位置
    startPosition = {0, 0};
    knightPosition = startPosition;
    std::cout << "Successfully loaded knight.png\n";
}

void ChessboardRenderer::setStartPosition(int x, int y) {
    if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE) {
        startPosition = {x, y};
        reset();
    }
}

void ChessboardRenderer::reset() {
    // 重置棋盘状态
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            boardState[i][j] = 0;
        }
    }
    
    knightPosition = startPosition;
    boardState[startPosition.x][startPosition.y] = 1;
    
    currentStep = 0;
    currentSolution = 0;
    animProgress = 0.0f;
    animationTime = 0.0f;
    running = false;
    paused = false;
    completed = false;
    statusMessage = "";
}

void ChessboardRenderer::runAlgorithm(Algorithm algo) {
    if (running) return;
    
    // 重置状态
    reset();
    
    // 运行算法
    solutions = solve(algo, startPosition);
    
    if (solutions.empty()) {
        statusMessage = "No solution found!";
        return;
    }
    
    running = true;
    paused = false;
    currentStep = 0;
    currentSolution = 0;
    animStartPos = startPosition;
    
    // 第一步已经设置好了（起点），所以需要将animEndPos设置为第一步的终点
    if (solutions[0].size() > 1) {
        animEndPos = solutions[0][1].end;
    } else {
        animEndPos = startPosition;
    }
    
    statusMessage = "Running...";
}

void ChessboardRenderer::update(float deltaTime) {
    if (!running || paused || completed) return;
    
    animationTime += deltaTime * animationSpeed;
    
    if (animationTime >= stepDuration) {
        // 进入下一步
        animationTime = 0.0f;
        processNextStep();
    } else {
        // 更新动画进度
        animProgress = animationTime / stepDuration;
    }
}

void ChessboardRenderer::processNextStep() {
    if (currentSolution >= solutions.size()) {
        completed = true;
        statusMessage = "All solutions completed!";
        return;
    }
    
    const Path& currentPath = solutions[currentSolution];
    
    if (currentStep >= currentPath.size()) {
        // 当前解决方案已完成
        if (currentSolution + 1 < solutions.size()) {
            // 还有更多解决方案
            currentSolution++;
            currentStep = 0;
            reset();
            statusMessage = "Solution " + std::to_string(currentSolution + 1) + " of " + 
                           std::to_string(solutions.size());
            return;
        } else {
            // 所有解决方案都完成了
            completed = true;
            statusMessage = "All solutions completed!";
            return;
        }
    }
    
    // 处理当前步骤
    const Arrow& arrow = currentPath[currentStep];
    
    if (arrow.stepNext) {
        // 前进
        knightPosition = arrow.end;
        int step = 0;
        
        // 找出当前是第几步
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (boardState[i][j] > step) {
                    step = boardState[i][j];
                }
            }
        }
        
        boardState[knightPosition.x][knightPosition.y] = step + 1;
    } else {
        // 回溯
        knightPosition = arrow.end;
        boardState[arrow.start.x][arrow.start.y] = 0;
    }
    
    currentStep++;
    
    // 设置下一步的动画起点和终点
    animStartPos = knightPosition;
    if (currentStep < currentPath.size()) {
        if (currentPath[currentStep].stepNext) {
            animEndPos = currentPath[currentStep].end;
        } else {
            animEndPos = currentPath[currentStep].end;
        }
    }
    
    animProgress = 0.0f;
    
    // 检查是否找到完整解决方案
    bool foundComplete = true;
    for (int i = 0; i < BOARD_SIZE && foundComplete; i++) {
        for (int j = 0; j < BOARD_SIZE && foundComplete; j++) {
            if (boardState[i][j] == 0) {
                foundComplete = false;
            }
        }
    }
    
    if (foundComplete) {
        paused = true;
        statusMessage = "Solution found! Step: " + std::to_string(currentStep);
    }
}

void ChessboardRenderer::togglePause() {
    paused = !paused;
    statusMessage = paused ? "Paused" : "Running...";
}

void ChessboardRenderer::setSpeed(float speed) {
    animationSpeed = speed;
}

void ChessboardRenderer::nextStep() {
    if (!running) return;
    
    animationTime = 0.0f;
    animProgress = 0.0f;
    processNextStep();
}

void ChessboardRenderer::prevStep() {
    // 回到上一步的逻辑（如果需要的话）
    // 这个功能比较复杂，需要记录每一步的棋盘状态
    // 暂时可以不实现
}

void ChessboardRenderer::drawBoard(sf::RenderWindow& window) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
            cell.setPosition({BOARD_OFFSET_X + j * CELL_SIZE, BOARD_OFFSET_Y + i * CELL_SIZE});
            
            // 黑白相间
            if ((i + j) % 2 == 0) {
                cell.setFillColor(sf::Color(240, 217, 181)); // 浅色
            } else {
                cell.setFillColor(sf::Color(181, 136, 99)); // 深色
            }
            
            window.draw(cell);
            
            // 绘制网格坐标
            sf::Text coordText(font);
            coordText.setString(std::to_string(i) + "," + std::to_string(j));
            coordText.setCharacterSize(10);
            coordText.setFillColor(sf::Color(100, 100, 100, 128));
            coordText.setPosition({BOARD_OFFSET_X + j * CELL_SIZE + 2, 
                                   BOARD_OFFSET_Y + i * CELL_SIZE + 2});
            window.draw(coordText);
        }
    }
}

void ChessboardRenderer::drawNumbers(sf::RenderWindow& window) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (boardState[i][j] > 0) {
                sf::Text stepText(font);
                stepText.setString(std::to_string(boardState[i][j]));
                stepText.setCharacterSize(16);
                stepText.setFillColor(sf::Color::Black);
                
                // 居中显示
                sf::FloatRect textBounds = stepText.getLocalBounds();
                stepText.setOrigin({textBounds.position.x + textBounds.size.x/2.0f,
                                  textBounds.position.y + textBounds.size.y/2.0f});
                                  
                stepText.setPosition({BOARD_OFFSET_X + j * CELL_SIZE + CELL_SIZE/2.0f,
                                    BOARD_OFFSET_Y + i * CELL_SIZE + CELL_SIZE/2.0f});
                window.draw(stepText);
            }
        }
    }
}

void ChessboardRenderer::drawKnight(sf::RenderWindow& window) {
    // 计算马的插值位置
    float x, y;
    
    if (running && !paused && !completed && animProgress < 1.0f) {
        // 动画中
        x = animStartPos.y * CELL_SIZE + BOARD_OFFSET_X + CELL_SIZE/2.0f + 
            (animEndPos.y - animStartPos.y) * CELL_SIZE * animProgress;
        y = animStartPos.x * CELL_SIZE + BOARD_OFFSET_Y + CELL_SIZE/2.0f + 
            (animEndPos.x - animStartPos.x) * CELL_SIZE * animProgress;
    } else {
        // 静止状态
        x = knightPosition.y * CELL_SIZE + BOARD_OFFSET_X + CELL_SIZE/2.0f;
        y = knightPosition.x * CELL_SIZE + BOARD_OFFSET_Y + CELL_SIZE/2.0f;
    }
    
    knight.setPosition({x, y});
    window.draw(knight);
}

void ChessboardRenderer::drawPath(sf::RenderWindow& window) {
    if (!running) return;
    
    const Path& currentPath = solutions[currentSolution];
    
    // 绘制已经走过的路径
    for (int i = 1; i <= currentStep && i < currentPath.size(); i++) {
        const Arrow& arrow = currentPath[i-1];
        if (arrow.stepNext) {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(
                    BOARD_OFFSET_X + arrow.start.y * CELL_SIZE + CELL_SIZE/2.0f,
                    BOARD_OFFSET_Y + arrow.start.x * CELL_SIZE + CELL_SIZE/2.0f), 
                    sf::Color(0, 100, 255, 128)),
                sf::Vertex(sf::Vector2f(
                    BOARD_OFFSET_X + arrow.end.y * CELL_SIZE + CELL_SIZE/2.0f,
                    BOARD_OFFSET_Y + arrow.end.x * CELL_SIZE + CELL_SIZE/2.0f), 
                    sf::Color(0, 100, 255, 128))
            };
            
            window.draw(line, 2, sf::PrimitiveType::Lines);
        }
    }
}

void ChessboardRenderer::render(sf::RenderWindow& window) {
    drawBoard(window);
    drawPath(window);
    drawNumbers(window);
    drawKnight(window);
    
    // 绘制状态消息
    sf::Text status(font);
    status.setString(statusMessage);
    status.setCharacterSize(18);
    status.setFillColor(sf::Color::Black);
    status.setPosition({BOARD_OFFSET_X, 
                      BOARD_OFFSET_Y + BOARD_SIZE * CELL_SIZE + 20});
    window.draw(status);
}