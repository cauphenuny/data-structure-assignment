#pragma once

#include <cstdint>

enum class Status : uint8_t {
    SUCCESS = 0,  // success value must be 0
    FAILED,
};

enum Dir : uint8_t {
    L = 0,
    R = 1,
};
