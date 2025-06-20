#pragma once

#include "util.hpp"

#include <iostream>

namespace trait {

template <typename Tree> struct Print {
    void printCLI() const {
        auto& self = *(static_cast<const Tree*>(this));
        if (!self.root) {
            std::cout << "Tree is empty." << '\n';
            return;
        }
        auto print_node = [&](auto self, auto node, int depth) {
            if (!node) return;
            self(self, node->child[L].get(), depth + 1);
            std::cout << std::string(depth * 4, ' ') << node->key << ": " << node->value << "\n";
            self(self, node->child[R].get(), depth + 1);
        };
        print_node(print_node, self.root.get(), 0);
    }
    void print() const {
        // auto& self = *(static_cast<const Tree*>(this));
        // TODO:
    }
};

}  // namespace trait
