/// @file gui.h
/// @brief GUI functions

#include <string_view>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR,
    DEBUG,
};

void echo(std::string_view message, LogLevel level = LogLevel::INFO);
