#import "@preview/touying:0.6.1": *
#import themes.university: *

== 算法

=== 暴力搜索算法

暴力算法实现 `solve_brute_force`(`Point start`)：

通过手动维护一个栈记录走过的路径结点坐标与正在尝试的方向，当无路可走时利用栈进行回溯从而尝试新的路径。

```cpp
    class SimpleStack {                            struct Node {
    private:                                           Point pos;  // 当前坐标
        T* base;                                       int move_index;  // 当前正在尝试的方向
        int top;                                   };
        int capacity;  // 当前容量

        void expand() { ...
    public:
        SimpleStack(int initial_capacity = 100) : base(nullptr), top(-1), capacity(initial_capacity)    { ...                             // 构造
        ~SimpleStack() { ...            // 销毁
        void push(const T& value) { ... // 入栈
        void pop() { ...                // 出栈
        T& peek() { ...                 // 读栈顶
        bool empty() const { ...        // 判断是否栈空
        int size() const { ...          // 返回栈元素个数
```

---

为了可视化搜索的过程，我们决定记录过程中每一步试探（包括回溯）。

最开始算法中每步都存储完整的棋盘（二维数组），但每步都存储一个棋盘带来的内存占用过大。

后来我们决定只使用起始点与终止点的坐标对记录每一步的行动，通过 `stepNext` 标签记录该步是前进还是回溯。

```cpp
    struct Arrow {
        Point start, end;
        bool stepNext; // 前进为1，后退为0
    };

    void Print_board(Board);

    using Path = std::vector<Arrow>; // 一条可行路径

    // 算法返回值：
    return std::vector<Path> //允许返回多条可行路径
```

但即便如此，暴力搜索算法的大量路径试探仍会带来无法承受的内存开销。

---
=== 贪心算法
贪心算法实现 `solve_heuristic`(`Point start`)：

基于 `H. C. von Warnsdorf` 于1823提出的 `Warnsdorf's Rule` ——每步选择可移动方向最少得位置移动。该算法可以以极快的速度给出一个可行解。

通过 `count_onward_moves()` 计算落点的可走步数：

```cpp
    static int count_onward_moves(const Board& board, int x, int y) {
        int count = 0;
        for (int i = 0; i < 8; ++i) {
              int nx = x + dx[i];
              int ny = y + dy[i];
              if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board(nx, ny) == 0) {
                  ++count;
              }
        }
        return count;
    }
```
---

在`solve_heuristic`(`Point start`) 内部对每一步的 `MoveOption` 数组排序，并向最小的方向进发。

```cpp
    // 枚举所有下一步的可选走法
    for (int i = 0; i < 8; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board(nx, ny) == 0) {
            int onward = count_onward_moves(board, nx, ny);
            options.push_back({nx, ny, onward});
        }
    }
    // 按后继步数升序排序
    std::sort(options.begin(), options.end(), [](const MoveOption& a, const MoveOption& b) {
        return a.onward < b.onward;
    });
```
但贪心算法只能得到一个可行解。我们希望算法可以找到多条可行解（具有找到全部可行解的潜力）。

---

=== 基于`Warnsdorf's Rule`的深度搜索算法
算法实现 `solve_heuristic_enhancer`(`Point start`)：

首次到达某结点时，基于`Warnsdorf's Rule`对其可行方向排序，优先选择出路最少的方向从而减少回溯次数。

由于框架仍是深度优先搜索，如果想找到多条可行路径，只需在找到一条可行路径后继续回溯即可。

```cpp
    while (!stk.empty()) {
        if (step == BOARD_SIZE * BOARD_SIZE) {
            if (!stk.empty()) {...} // 保存最后一步结果
            if(countHistory == NUM_OF_PATH) break;
            else {// 后退一步继续搜其他路径
                Point end = stk.peek().pos;
                stk.pop();  --step; board(end.x, end.y) = 0;
                Point start = stk.empty() ? end : stk.peek().pos;
                history.push_back({end, start, 0});
                continue;
            }
    }
```
