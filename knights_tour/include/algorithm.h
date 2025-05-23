/// @file algorithm.h

#pragma once

#include "board.h"

enum class Algorithm {
    BRUTE_FORCE,  // 暴力
    HEURISTIC,    // 启发式搜索
};

void solve(Algorithm algo, Point start);
