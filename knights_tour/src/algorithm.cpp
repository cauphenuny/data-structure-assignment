#include "algorithm.h"

#include <algorithm>
#include <array>
#include <stack>
#include <utility>

#define NUM_OF_PATH 2 // 搜索路径数

const std::array<int, 8> dx = {1, 2, 2, 1, -1, -2, -2, -1};
const std::array<int, 8> dy = {2, 1, -1, -2, -2, -1, 1, 2};

std::vector<Path> solve(Algorithm algo, Point start) {
    switch (algo) {
        case Algorithm::BRUTE_FORCE: return solve_brute_force(start);  // 调用暴力算法实现
        case Algorithm::HEURISTIC: return solve_heuristic(start);      // 调用启发式算法实现
        case Algorithm::HEURISTIC_INHANCER: return solve_heuristic_inhancer(start); // 启发式排序的深度优先搜索
        default:
            throw std::invalid_argument("Unknown algorithm type");
    }
}

std::vector<Path> solve_brute_force(Point start) {
    struct Node {
        Point pos;
        int move_index;  // 当前正在尝试的方向
    };

    Board board;
    // 初始化棋盘未访问状态
    for (int i = 0; i < BOARD_SIZE; ++i)
        for (int j = 0; j < BOARD_SIZE; ++j) board(i, j) = 0;

    SimpleStack<Node> stk;;
    Path history; // 当前路径
    std::vector<Path> result;

    int step = 1;
    board(start.x, start.y) = step;
    stk.push({start, 0});
    history.push_back({{}, {}, 1});

    while (!stk.empty()) {
        if (step == BOARD_SIZE * BOARD_SIZE) {
            result.push_back({history}) ;
            break;// //
            // 回退一步
            /*
            Node current = stk.top();
            stk.pop();
            board(current.pos.x, current.pos.y) = 0;
            step--;
            history.push_back({board, {}});
            continue;
            */
        }

        Node& current = stk.peek();
        bool moved = false;

        for (; current.move_index < 8; ++current.move_index) {
            int nx = current.pos.x + dx[current.move_index];
            int ny = current.pos.y + dy[current.move_index];

            if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board(nx, ny) == 0) {
                // 有效位置且未访问，走下一步
                ++step;
                board(nx, ny) = step;
                stk.push({{nx, ny}, 0});

                // 保存状态
                Arrow arrow = {current.pos, {nx, ny}, 1};
                history.push_back({arrow});

                moved = true;
                break;
            }
        }

        if (!moved) {
            Point end = current.pos;
            stk.pop();
            --step;

            Point start = stk.empty() ? end : stk.peek().pos;
            board(end.x, end.y) = 0;
            history.push_back({end, start, 0}); // 反向记录回退路径
        }
    }

    // 如果失败返回一个空棋盘（全 0）
    return result;
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

std::vector<Path> solve_heuristic(Point start) {
    Board board;
    Path history;
    
    for (int i = 0; i < BOARD_SIZE; ++i)
        for (int j = 0; j < BOARD_SIZE; ++j) board(i, j) = 0;

    int step = 1;
    int x = start.x;
    int y = start.y;
    board(x, y) = step;

    history.push_back({{}, {}, 1});

    while (step < BOARD_SIZE * BOARD_SIZE) {
        struct MoveOption {
            int nx, ny;
            int onward;  // 后继可选步数
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
            // 无法继续前进
            throw "Cannot move";
        }

        // 按后继步数升序排序
        std::sort(options.begin(), options.end(), [](const MoveOption& a, const MoveOption& b) {
            return a.onward < b.onward;
        });

        // 选择最优方向
        int next_x = options[0].nx;
        int next_y = options[0].ny;

        Arrow move = {{x, y}, {next_x, next_y}, 1};
        x = next_x;
        y = next_y;
        ++step;
        board(x, y) = step;

        history.push_back({move});
    }
    std::vector<Path> result;
    result.push_back(history);
    return result;
}

std::vector<Path> solve_heuristic_inhancer(Point start) {
    struct Node {
        Point pos;
        int move_index;  // 当前正在尝试的方向索引
        std::vector<int> sorted_dirs;  // 启发式排序后的方向索引数组
    };

    Board board;
    for (int i = 0; i < BOARD_SIZE; ++i)
        for (int j = 0; j < BOARD_SIZE; ++j)
            board(i, j) = 0;

    SimpleStack<Node> stk;
    Path history;
    std::vector<Path> result;

    int step = 1;
    board(start.x, start.y) = step;
    stk.push({start, 0, {}});
    history.push_back({{}, {}, 1});
    int countHistory = 0;

    while (!stk.empty()) {
        if (step == BOARD_SIZE * BOARD_SIZE) {
            if (!stk.empty()) {
                Arrow final = {stk.peek().pos, stk.peek().pos, 1}; // 保存最后一格
                history.push_back({final});
            }
            result.push_back(history);
            countHistory++;
            if(countHistory == NUM_OF_PATH)
                break;
            else {
                // 后退一步继续搜其他路径
                Point end = stk.peek().pos;
                stk.pop();
                --step;
                board(end.x, end.y) = 0;

                Point start = stk.empty() ? end : stk.peek().pos;
                history.push_back({end, start, 0});
                continue;
            }
        }

        Node& current = stk.peek();
        Point cur_pos = current.pos;

        // 首次处理该节点时，按启发式规则排序可能方向
        if (current.sorted_dirs.empty()) {
            struct MoveOption {
                int dir;
                int onward;
            };
            std::vector<MoveOption> options;
            for (int i = 0; i < 8; ++i) {
                int nx = cur_pos.x + dx[i];
                int ny = cur_pos.y + dy[i];
                if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board(nx, ny) == 0) {
                    int onward = count_onward_moves(board, nx, ny);
                    options.push_back({i, onward});
                }
            }

            std::sort(options.begin(), options.end(), [](const MoveOption& a, const MoveOption& b) {
                return a.onward < b.onward;
            });

            for (const auto& opt : options)
                current.sorted_dirs.push_back(opt.dir);
        }

        bool moved = false;

        while (current.move_index < static_cast<int>(current.sorted_dirs.size())) {
            int dir = current.sorted_dirs[current.move_index++];
            int nx = cur_pos.x + dx[dir];
            int ny = cur_pos.y + dy[dir];

            if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board(nx, ny) == 0) {
                ++step;
                board(nx, ny) = step;
                stk.push({{nx, ny}, 0, {}});
                history.push_back({cur_pos, {nx, ny}, 1});
                moved = true;
                break;
            }
        }

        if (!moved) {
            Point end = cur_pos;
            stk.pop();
            --step;
            board(end.x, end.y) = 0;

            Point prev = stk.empty() ? end : stk.peek().pos;
            history.push_back({end, prev, 0});
        }
    }

    return result;
}