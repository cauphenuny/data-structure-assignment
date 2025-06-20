#include "board.h"
#include <iostream>

void Board::print_board() {

    for(int i = 0; i < BOARD_SIZE; ++i) {
        for(int j = 0; j < BOARD_SIZE; ++j)
            std::cout << data[i][j] << " ";
        std::cout << std::endl;
    }
}