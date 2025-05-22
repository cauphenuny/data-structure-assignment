#pragma once

#include "board.h"

typedef enum {
    INFO,
    WARN,
    ERROR,
} level_t;

void echo_message(level_t level, const char* fmt, ...);

void draw_board(const board_t board);
void draw_arrow(point_t start, point_t end);
void clear_canvas();
void clear_arrow();
