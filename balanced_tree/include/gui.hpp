/// @file gui.h
/// @brief GUI functions

#pragma once
#include "tree/interface.hpp"

#include <cstdint>
#include <string_view>

/****************************** Definition ********************************/

enum class LogLevel : uint8_t {
    INFO,
    WARNING,
    ERROR,
    DEBUG,
};

void echo(std::string_view message, LogLevel level = LogLevel::INFO);

void gui();

void drawTree(TreeBase* tree);

// NOTE: don't forget to implement Tree<Key, Value>::print()
