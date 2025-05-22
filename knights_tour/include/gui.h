#pragma once

#include "board.h"

#include <string_view>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR,
    DEBUG,
};

void echo_message(std::string_view message, LogLevel level = LogLevel::INFO);

void draw_board(const Board& board);
void draw_arrow(Point start, Point end);
void clear_canvas();
void clear_arrow();
