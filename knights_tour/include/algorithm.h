/// @file algorithm.h

#pragma once

#include "board.h"
#include <stdexcept>
#include <cstdint>

enum class Algorithm : uint8_t {
    BRUTE_FORCE,  // 暴力
    HEURISTIC,    // 启发式搜索
    HEURISTIC_INHANCER  // 启发式搜索返回多条路径
};

template<typename T>    // 手动维护的栈
class SimpleStack {
private:
    T* base;       // 动态数组指针
    int top;       // 栈顶索引，-1表示空栈
    int capacity;  // 当前容量

    void expand() {
        int new_capacity = capacity * 2;
        T* new_base = new T[new_capacity];
        for (int i = 0; i <= top; ++i) {
            new_base[i] = base[i];
        }
        delete[] base;
        base = new_base;
        capacity = new_capacity;
    }

public:
    SimpleStack(int initial_capacity = 100) : base(nullptr), top(-1), capacity(initial_capacity) {
        base = new T[capacity];
    }

    ~SimpleStack() {
        delete[] base;
    }

    void push(const T& value) {
        if (top + 1 == capacity) {
            expand();
        }
        base[++top] = value;
    }

    void pop() {
        if (top == -1) throw std::out_of_range("Stack is empty");
        --top;
    }

    T& peek() {
        if (top == -1) throw std::out_of_range("Stack is empty");
        return base[top];
    }

    bool empty() const {
        return top == -1;
    }

    int size() const {
        return top + 1;
    }
};

std::vector<Path> solve(Algorithm algo, Point start);

std::vector<Path> solve_brute_force(Point start);

std::vector<Path> solve_heuristic(Point start);

std::vector<Path> solve_heuristic_inhancer(Point start);