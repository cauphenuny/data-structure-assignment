/// @file board.h
/// @brief define the board structure

#pragma once
#include <vector>

#define BOARD_SIZE 8

struct Point {
    int x, y;
};

struct Board {
    int data[BOARD_SIZE][BOARD_SIZE];
    int& operator()(int x, int y) { return data[x][y]; }
    const int& operator()(int x, int y) const { return data[x][y]; }
};

/*
 * usage:
 *
 * Board board;
 * board(1, 2) = 1; // same as board.data[1][2] = 1;
 * board(3, 2) = 1;
 */

struct Arrow {
    Point start, end;
};

struct DisplayBoard {
    Board board;
    std::vector<Arrow> arrows;
};
