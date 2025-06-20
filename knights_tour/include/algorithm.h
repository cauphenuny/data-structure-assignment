/// @file algorithm.h

#pragma once

#include "board.h"
#include <cstdint>

enum class Algorithm : uint8_t {
    BRUTE_FORCE,  // 暴力
    HEURISTIC,    // 启发式搜索
};

Board solve(Algorithm algo, Point start);

Board solve_brute_force(Point start);

Board solve_heuristic(Point start);