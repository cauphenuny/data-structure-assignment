#include "tree/avl.hpp"
#include "tree/basic.hpp"
#include "tree/splay.hpp"
#include "tree/treap.hpp"

#include <cstdint>
#include <format>
#include <iostream>
#include <map>

void printNodeViewCLI(const NodeView* node, int depth) {
    if (!node) return;
    printNodeViewCLI(node->child[R].get(), depth + 1);
    auto [key, value] = node->content();
    std::cout << std::format("{}{{{}: {}}}\n", std::string(depth * 4, ' '), key, value);
    printNodeViewCLI(node->child[L].get(), depth + 1);
}

void printForestCLI(const ForestView& forest) {
    for (size_t i = 0; auto& node : forest) {
        if (node) {
            if (i++) std::cout << "----\n";
            printNodeViewCLI(node.get(), 0);
        }
    }
}

void printTraceCLI(
    std::string_view title, std::string_view usage, const std::vector<ForestView>& trace) {
    std::cout << title << ":\n";
    int auto_play = 0;
    for (size_t i = 0; i < trace.size(); ++i) {
        std::cout << "#" << i + 1 << ":\n";
        printForestCLI(trace[i]);
        if (i < trace.size() - 1) {
            std::cout << "------------\n";
            if (!auto_play) {
                char ch;
                do {
                    printf("(trace) ");
                    std::string buffer;
                    std::getline(std::cin, buffer);
                    ch = buffer.empty() ? 'n' : buffer[0];
                } while (ch != 'n' && ch != 'c' && ch != 'h');
                if (ch == 'c') auto_play = 1;
                if (ch == 'h') {
                    std::cout << usage;
                }
            }
        }
    }
}

int runCLI() {
    using K = int;
    const std::string K_NAME = "int";
    using V = int;
    const std::string V_NAME = "int";
    enum Algorithm : int8_t { BASIC, AVL, TREAP, SPLAY };
    struct TreeInfo {
        Algorithm algo;
        Tree<K, V>* tree{nullptr};
    };
    std::map<char, TreeInfo> viewers;
    std::map<char, std::unique_ptr<BasicTree<K, V>>> basics;
    std::map<char, std::unique_ptr<AVLTree<K, V>>> avls;
    std::map<char, std::unique_ptr<Treap<K, V>>> treaps;
    std::map<char, std::unique_ptr<SplayTree<K, V>>> splays;
    auto refresh = [&] {
        viewers.clear();
        for (auto& [ch, tree] : basics) {
            viewers[ch] = {Algorithm::BASIC, tree.get()};
        }
        for (auto& [ch, tree] : avls) {
            viewers[ch] = {Algorithm::AVL, tree.get()};
        }
        for (auto& [ch, tree] : treaps) {
            viewers[ch] = {Algorithm::TREAP, tree.get()};
        }
        for (auto& [ch, tree] : splays) {
            viewers[ch] = {Algorithm::SPLAY, tree.get()};
        }
    };
    std::string usage = std::format(
        R"(commands:
    [q]uit
    [h]elp

    [c]reate <tree-id: a-z|A-Z> <algo: basic|avl|treap|splay>
    [d]elete <tree-id>
    [p]rint <tree-id>*
    [l]ist

    [i]nsert <tree-id> <key: {0}> <value: {1}>
    [r]emove <tree-id> <key: {0}>
    [f]ind <tree-id> <key: {0}>

    [s]plit <dest-id> <src-id> <key: {0}>
    [m]erge <dest-id> <src-id>

    [R]andom-insert <tree-id> <count: int>
    [S]equential-insert <tree-id> <start: {0}> <end: {0}>

trace mode:
    [n]: next
    [c]: auto continue
)",
        K_NAME, V_NAME);
    std::cout << usage;
    enum Ret : int8_t { INVALID, COMSUMED, EXIT };
    auto print_trace = [&](int index) {
        auto trace = viewers[index].tree->trace();
        printTraceCLI(std::format("------------\nTrace of tree {}", (char)index), usage, trace);
    };
    std::map<char, std::function<Ret(std::istringstream)>> commands = {
        {'q', [](std::istringstream) -> Ret { return Ret::EXIT; }},
        {'h',
         [&](std::istringstream) -> Ret {
             std::cout << usage;
             return Ret::COMSUMED;
         }},
        {'l',
         [&](std::istringstream) -> Ret {
             if (viewers.empty()) {
                 std::cout << "No trees initialized.\n";
             } else {
                 std::cout << "Initialized trees:\n";
                 for (const auto& [index, info] : viewers) {
                     if (info.tree)
                         std::cout << std::format("Tree {}: {}\n", index, info.tree->name());
                 }
             }
             return Ret::COMSUMED;
         }},
        {'c',
         [&](std::istringstream iss) -> Ret {
             char index = 0;
             std::string algo;
             iss >> index >> algo;
             if (!isalpha(index)) return Ret::INVALID;
             if (viewers[index].tree) {
                 std::cout << std::format("Tree {} already exists.\n", index);
                 return Ret::COMSUMED;
             }
             if (algo == "basic") {
                 basics[index] = std::make_unique<BasicTree<K, V>>();
             } else if (algo == "avl") {
                 avls[index] = std::make_unique<AVLTree<K, V>>();
             } else if (algo == "treap") {
                 treaps[index] = std::make_unique<Treap<K, V>>();
             } else if (algo == "splay") {
                 splays[index] = std::make_unique<SplayTree<K, V>>();
             } else {
                 std::cout << "Unknown algorithm: " << algo << '\n';
                 return Ret::INVALID;
             }
             refresh();
             viewers[index].tree->traceStart();
             std::cout << std::format("Created tree {} with algorithm {}\n", index, algo);
             return Ret::COMSUMED;
         }},
        {'p',
         [&](std::istringstream iss) -> Ret {
             std::string name;
             iss >> name;
             auto print = [&](char index) {
                 if (!viewers[index].tree) {
                     std::cout << std::format("Tree {}: not initialized.\n", index);
                     return;
                 }
                 std::cout << std::format("Tree {}: {}:\n", index, viewers[index].tree->name());
                 viewers[index].tree->printCLI(1);
             };
             if (name.empty()) {
                 for (int i = 0; i < 256; i++) {
                     if (isalpha(i) && viewers[i].tree) {
                         print(i);
                     }
                 }
             } else {
                 for (auto ch : name) {
                     if (isalpha(ch)) {
                         print(ch);
                     }
                 }
             }
             return Ret::COMSUMED;
         }},
        {'i',
         [&](std::istringstream iss) -> Ret {
             char index = 0;
             K key;
             V value;
             iss >> index >> key >> value;
             if (!isalpha(index)) return Ret::INVALID;
             if (viewers[index].tree) {
                 viewers[index].tree->insert(key, value);
                 std::cout << std::format("Inserted {{{}: {}}} into tree {}\n", key, value, index);
                 print_trace(index);
             } else {
                 std::cout << std::format("Tree {} not initialized.\n", index);
             }
             return Ret::COMSUMED;
         }},
        {'r',
         [&](std::istringstream iss) -> Ret {
             char index = 0;
             K key;
             iss >> index >> key;
             if (!isalpha(index)) return Ret::INVALID;
             if (viewers[index].tree && viewers[index].tree->remove(key) == Status::SUCCESS) {
                 std::cout << std::format("Removed {} from tree {}\n", key, index);
                 print_trace(index);
             } else {
                 std::cout << std::format("Failed to remove {} from tree {}\n", key, index);
             }
             return Ret::COMSUMED;
         }},
        {'f',
         [&](std::istringstream iss) -> Ret {
             char index = 0;
             K key;
             iss >> index >> key;
             if (!isalpha(index)) return Ret::INVALID;
             if (viewers[index].tree) {
                 auto pair = viewers[index].tree->find(key);
                 if (pair) {
                     std::cout << std::format(
                         "Found {{{}: {}}} in tree {}\n", pair->key, pair->value, index);
                 } else {
                     std::cout << std::format("Key {} not found in tree {}\n", key, index);
                 }
             } else {
                 std::cout << std::format("Tree {} not initialized.\n", index);
             }
             return Ret::COMSUMED;
         }},
        {
            's',
            [&](std::istringstream iss) -> Ret {
                char dest = 0, src = 0;
                K key;
                iss >> dest >> src >> key;
                if (!isalpha(dest) || !isalpha(src)) return Ret::INVALID;
                if (viewers[dest].tree) {
                    std::cout << std::format("Tree {} already exists.\n", dest);
                }
                if (!viewers[src].tree) {
                    std::cout << std::format("Tree {} not initialized.\n", src);
                    return Ret::INVALID;
                }
                viewers[dest].algo = viewers[src].algo;
                switch (viewers[src].algo) {
                    case Algorithm::BASIC: {
                        basics[dest] = basics[src]->split(key);
                        break;
                    }
                    case Algorithm::AVL: {
                        avls[dest] = avls[src]->split(key);
                        break;
                    }
                    case Algorithm::TREAP: {
                        treaps[dest] = treaps[src]->split(key);
                        break;
                    }
                    case Algorithm::SPLAY: {
                        splays[dest] = splays[src]->split(key);
                        break;
                    }
                }
                refresh();
                print_trace(src);
                viewers[dest].tree->traceStart();
                return Ret::COMSUMED;
            },
        },
        {'m',
         [&](std::istringstream iss) -> Ret {
             char dest = 0, src = 0;
             iss >> dest >> src;
             if (!isalpha(dest) || !isalpha(src)) return Ret::INVALID;
             if (!viewers[dest].tree || !viewers[src].tree) {
                 std::cout << "Both trees must be initialized.\n";
                 return Ret::INVALID;
             }
             if (viewers[dest].algo != viewers[src].algo) {
                 std::cout << "Cannot merge trees with different algorithms.\n";
                 return Ret::INVALID;
             }
             switch (viewers[dest].algo) {
                 case Algorithm::BASIC: basics[dest]->merge(std::move(basics[src])); break;
                 case Algorithm::AVL: avls[dest]->merge(std::move(avls[src])); break;
                 case Algorithm::TREAP: treaps[dest]->merge(std::move(treaps[src])); break;
                 case Algorithm::SPLAY: splays[dest]->merge(std::move(splays[src])); break;
             }
             refresh();
             print_trace(dest);
             return Ret::COMSUMED;
         }},
        {'d',
         [&](std::istringstream iss) -> Ret {
             char index = 0;
             iss >> index;
             if (!isalpha(index)) return Ret::INVALID;
             if (viewers[index].tree) {
                 viewers[index].tree->traceStop();
                 switch (viewers[index].algo) {
                     case Algorithm::BASIC: basics.erase(index); break;
                     case Algorithm::AVL: avls.erase(index); break;
                     case Algorithm::TREAP: treaps.erase(index); break;
                     case Algorithm::SPLAY: splays.erase(index); break;
                 }
                 std::cout << std::format("Deleted tree {}\n", index);
             } else {
                 std::cout << std::format("Tree {} not initialized.\n", index);
             }
             refresh();
             return Ret::COMSUMED;
         }},
        {'R',
         [&](std::istringstream iss) -> Ret {
             char index = 0;
             int count = 0;
             iss >> index >> count;
             if (!isalpha(index)) return Ret::INVALID;
             if (count <= 0) return Ret::INVALID;
             int range = count * 10;
             if (viewers[index].tree) {
                 auto& tree = viewers[index].tree;
                 for (int i = 0; i < count; ++i) {
                     K key = rand() % range;    // Random key
                     V value = rand() % range;  // Random value
                     tree->insert(key, value);
                 }
                 std::cout << std::format(
                     "Inserted {} random elements into tree {}\n", count, index);
                 print_trace(index);
             } else {
                 std::cout << std::format("Tree {} not initialized.\n", index);
             }
             return Ret::COMSUMED;
         }},
        {'S',
         [&](std::istringstream iss) -> Ret {
             char index = 0;
             int start = 0, end = 0;
             iss >> index >> start >> end;
             if (!isalpha(index)) return Ret::INVALID;
             if (start >= end) return Ret::INVALID;
             if (viewers[index].tree) {
                 int range = std::abs(std::max(start, end) * 10);
                 auto& tree = viewers[index].tree;
                 for (K i = start; i < end; ++i) {
                     tree->insert(i, rand() % range);  // Sequential values
                 }
                 std::cout << std::format(
                     "Inserted sequential elements from {} to {} into tree {}\n", start, end,
                     index);
                 print_trace(index);
             } else {
                 std::cout << std::format("Tree {} not initialized.\n", index);
             }
             return Ret::COMSUMED;
         }},
    };
    std::map<Ret, const char*> msg = {
        {Ret::INVALID, "Invalid command, type 'h' for help.\n"},
        {Ret::COMSUMED, ""},
        {Ret::EXIT, "Exit.\n"}};
    const char PROMPT[] = ">>> ";
    while (true) {
        std::cout << PROMPT;
        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << msg[Ret::EXIT];
            break;
        }
        std::istringstream iss(line);
        std::string cmd;
        if (!(iss >> cmd)) continue;
        if (commands.contains(cmd[0])) {
            auto ret = commands[cmd[0]](std::move(iss));
            std::cout << msg[ret];
            if (ret == Ret::EXIT) {
                break;
            }
        } else {
            std::cout << msg[Ret::INVALID];
        }
    }
    return 0;
}
