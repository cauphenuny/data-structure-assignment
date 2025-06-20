#include "algorithm.h"
#include <iostream>

int main() {
    Point start_p;
    int choose_modes;
    Algorithm modes;
    std::cin >> start_p.x >> start_p.y;
    std::cin >> choose_modes;
    modes = choose_modes == 0 ? Algorithm::BRUTE_FORCE : Algorithm::HEURISTIC;
    
    Board out_board;
    out_board = solve(modes, start_p);
    
    out_board.print_board();

    return 0;
}