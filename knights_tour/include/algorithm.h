/// @file algorithm.h

#pragma once

#include "board.h"
#include <cstdint>

enum class Algorithm : uint8_t {
    BRUTE_FORCE,  // 暴力
    HEURISTIC,    // 启发式搜索
    HEURISTIC_INHANCER  // 启发式搜索返回多条路径
};

std::vector<Path> solve(Algorithm algo, Point start);

std::vector<Path> solve_brute_force(Point start);

std::vector<Path> solve_heuristic(Point start);

std::vector<Path> solve_heuristic_inhancer(Point start);