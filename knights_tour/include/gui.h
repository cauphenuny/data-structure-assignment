/// @file gui.h

#pragma once

#include "board.h"

#include <string_view>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR,
    DEBUG,
};

void echo(std::string_view message, LogLevel level = LogLevel::INFO);

void drawBoard(const DisplayBoard& board);
void clearCanvas();
