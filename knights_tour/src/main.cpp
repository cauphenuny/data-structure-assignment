#include "algorithm.h"
#include "gui/gui.hpp"
#include <iostream>
#include <cstring>

int main(int argc, char* argv[]) {
    if (argc > 1 && strcmp(argv[1], "gui") == 0) {
        KnightsTourGUI gui;
        gui.run();
        return 0;
    }

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
        modes = Algorithm::HEURISTIC_ENHANCER;
        break;
    default:
        throw "unvalid modes";
        break;
    }
    
    std::vector<Path> results;
    results = solve(modes, start_p);
    
    // out_board.print_board();

    for(int i = 0; i < (int)results.size(); i++) {
        for(int j = 1; j < (int)results[i].size(); j++)
            std::cout << "(" << results[i][j].start.x << ","<< results[i][j].start.y << ")";
        std::cout << std::endl;
    }


    return 0;
}