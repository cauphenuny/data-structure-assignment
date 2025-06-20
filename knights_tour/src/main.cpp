#include "algorithm.h"
#include <iostream>

int main() {
    Point start_p;
    int choose_modes;
    Algorithm modes;
    std::cin >> start_p.x >> start_p.y;
    std::cin >> choose_modes;

    switch (choose_modes)
    {
    case 0:
        modes = Algorithm::BRUTE_FORCE;
        break;
    case 1:
        modes = Algorithm::HEURISTIC;
        break;
    case 2:
        modes = Algorithm::HEURISTIC_INHANCER;
        break;
    default:
        throw "unvalid modes";
        break;
    }
    
    std::vector<Path> results;
    results = solve(modes, start_p);
    
    // out_board.print_board();
    size_t num_paths = results.size();
    size_t num_arrows = results[0].size();
    for(int i = 0; i < (int)num_paths; i++) {
        for(int j = 1; j < (int)num_arrows; j++)
            std::cout << "(" << results[0][j].start.x << ","<< results[0][j].start.y << ")";
        std::cout << std::endl;
    }


    return 0;
}