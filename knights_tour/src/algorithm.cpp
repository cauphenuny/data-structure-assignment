#include "algorithm.h"
#include <stack>
#include <array>
#include <utility>
#include <algorithm> 
#include <stdexcept>

const std::array<int, 8> dx = {1, 2, 2, 1, -1, -2, -2, -1};
const std::array<int, 8> dy = {2, 1, -1, -2, -2, -1, 1, 2};

Board solve(Algorithm algo, Point start) {
    switch (algo) {
        case Algorithm::BRUTE_FORCE:
            return solve_brute_force(start);  // 调用暴力算法实现
        case Algorithm::HEURISTIC:
            return solve_heuristic(start);     // 调用启发式算法实现
        default:
            // 处理未知算法类型（可选）
            throw std::invalid_argument("Unknown algorithm type");
    }
}

Board solve_brute_force(Point start) {
    struct Node {
        Point pos;
        int move_index; // 当前正在尝试的方向
    };

    Board board;
    // 初始化棋盘未访问状态
    for (int i = 0; i < BOARD_SIZE; ++i)
        for (int j = 0; j < BOARD_SIZE; ++j)
            board(i, j) = 0;

    std::stack<Node> stk;
    int step = 1;
    board(start.x, start.y) = step;
    stk.push({start, 0});

    while (!stk.empty()) {
        if (step == BOARD_SIZE * BOARD_SIZE)
            return board; 

        Node& current = stk.top();
        bool moved = false;

        for (; current.move_index < 8; ++current.move_index) {
            int nx = current.pos.x + dx[current.move_index];
            int ny = current.pos.y + dy[current.move_index];

            if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board(nx, ny) == 0) {
                // 有效位置且未访问，走下一步
                ++step;
                board(nx, ny) = step;
                stk.push({{nx, ny}, 0});
                moved = true;
                break;
            }
        }

        if (!moved) {
            // 当前节点无路可走，回退
            board(current.pos.x, current.pos.y) = 0;
            stk.pop();
            --step;
        }
    }

    // 如果失败返回一个空棋盘（全 0）
    return board;
}

static int count_onward_moves(const Board& board, int x, int y) {
    int count = 0;
    for (int i = 0; i < 8; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board(nx, ny) == 0) {
            ++count;
        }
    }
    return count;
}

Board solve_heuristic(Point start) {
    Board board;
    
    for (int i = 0; i < BOARD_SIZE; ++i)
        for (int j = 0; j < BOARD_SIZE; ++j)
            board(i, j) = 0;

    int step = 1;
    int x = start.x;
    int y = start.y;
    board(x, y) = step;

    while (step < BOARD_SIZE * BOARD_SIZE) {
        struct MoveOption {
            int nx, ny;
            int onward; // 后继可选步数
        };

        std::vector<MoveOption> options;

        // 枚举所有下一步的可选走法
        for (int i = 0; i < 8; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board(nx, ny) == 0) {
                int onward = count_onward_moves(board, nx, ny);
                options.push_back({nx, ny, onward});
            }
        }

        if (options.empty()) {
            break; // 无法继续前进，失败（不应该发生）
        }

        // 按后继步数升序排序
        std::sort(options.begin(), options.end(), [](const MoveOption& a, const MoveOption& b) {
            return a.onward < b.onward;
        });

        // 选择最优方向
        x = options[0].nx;
        y = options[0].ny;
        ++step;
        board(x, y) = step;
    }

    return board;
}
