/// @file gui.hpp
/// @brief grafical user interface for the tree

#pragma once
#include "tree.hpp"

/****************************** Definition ********************************/

enum class LogLevel {
    INFO,
    WARNING,
    ERROR,
    DEBUG,
};

void echo_message(std::string_view message, LogLevel level = LogLevel::INFO);

/****************************** Implementation ********************************/

template <typename Key, typename Value> void draw_tree(const TreeObject<Key, Value>& tree);
