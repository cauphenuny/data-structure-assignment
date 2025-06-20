#include "algorithm.h"
#include <iostream>

int main() {
    Point start_p;
    int choose_modes;
    Algorithm modes;
    std::cin >> start_p.x >> start_p.y;
    std::cin >> choose_modes;
    modes = choose_modes == 0 ? Algorithm::BRUTE_FORCE : Algorithm::HEURISTIC;
    
    std::vector<Path> results;
    results = solve(modes, start_p);
    
    // out_board.print_board();
    size_t num_arrows = results[0].size();
    for(int i=1; i < num_arrows; i++)
        std::cout << "(" << results[0][i].start.x << ","<< results[0][i].start.y << ")";

    return 0;
}