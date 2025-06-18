#pragma once
#include "_legacy/avl.hpp"
#include "_legacy/tree.hpp"
#include "tree/avl.hpp"
#include "tree/basic.hpp"
#include "tree/splay.hpp"
#include "tree/treap.hpp"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <format>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

using namespace std;

inline void crtpBenchmark() {
    using Key = int;
    using Value = int;
    constexpr int N = 200000;

    vector<Key> keys(N);
    for (int i = 0; i < N; ++i) keys[i] = i;
    shuffle(keys.begin(), keys.end(), mt19937{random_device{}()});

    struct Metrics {
        double insert_time;
        double find_time;
        double remove_time;
    };

    auto bench = [&](auto&& tree, string_view name) -> Metrics {
        // Insert
        auto start = chrono::high_resolution_clock::now();
        for (auto k : keys) tree->insert(k, k);
        auto end = chrono::high_resolution_clock::now();
        double insert_time = chrono::duration<double, milli>(end - start).count();

        // Find
        start = chrono::high_resolution_clock::now();
        for (auto k : keys) volatile auto _ = tree->find(k);
        end = chrono::high_resolution_clock::now();
        double find_time = chrono::duration<double, milli>(end - start).count();

        // Remove
        start = chrono::high_resolution_clock::now();
        for (auto k : keys) {
            if constexpr (requires { tree->remove(k); }) {
                tree->remove(k);
            } else {
                tree->erase(k);
            }
        }
        end = chrono::high_resolution_clock::now();
        double remove_time = chrono::duration<double, milli>(end - start).count();

        cout << format(
            "{:<20} {:>12.2f} {:>12.2f} {:>12.2f}\n", name, insert_time, find_time, remove_time);

        return {insert_time, find_time, remove_time};
    };

    cout << format("\n===== Tree Implementation Comparison =====\n");
    cout << format("{:<20} {:>12} {:>12} {:>12}\n", "Tree", "Insert", "Find", "Remove");

    struct StdMapWrapper {
        void insert(const Key& key, const Value& value) { m[key] = value; }
        void remove(const Key& key) { m.erase(key); }
        auto find(const Key& key) { return m.find(key); }
        map<Key, Value> m;
    };

    auto old_time = bench(make_unique<legacy::AVLTree<Key, Value>>(), "legacy::AVLTree(ms)");
    auto new_time = bench(make_unique<AVLTree<Key, Value>>(), "AVLTree(ms)");
    bench(make_unique<StdMapWrapper>(), "std::map(ms)");

    auto relative = [](double old_time, double new_time) {
        return (old_time - new_time) / old_time * 100.0;
    };

    cout << format(
        "\n{:<20} {:>12.2f} {:>12.2f} {:>12.2f}\n", "CRTP Improvement(%)",
        relative(old_time.insert_time, new_time.insert_time),
        relative(old_time.find_time, new_time.find_time),
        relative(old_time.remove_time, new_time.remove_time));
}

inline void algorithmBenchmark() {
    cout << format("\n===== Tree Algorithm Comparison =====\n");
    using Key = int;
    using Value = int;
    constexpr int N = 200000;
    vector<Key> keys(N);
    for (int i = 0; i < N; ++i) keys[i] = i;
    shuffle(keys.begin(), keys.end(), mt19937{random_device{}()});

    auto duration = [](auto func) {
        auto start = chrono::high_resolution_clock::now();
        func();
        auto end = chrono::high_resolution_clock::now();
        return chrono::duration<double, milli>(end - start).count();
    };
    auto micro_duration = [](auto func) {
        auto start = chrono::high_resolution_clock::now();
        func();
        auto end = chrono::high_resolution_clock::now();
        return chrono::duration<double, micro>(end - start).count();
    };
    auto bench = [&](auto tree, string_view name, bool include_sequential = true) {
        double insert_time = duration([&] {
            for (auto k : keys) tree->insert(k, k);
        });
        double find_time = duration([&] {
            for (auto k : keys) volatile auto _ = tree->find(k);
        });
        double remove_time = duration([&] {
            for (auto k : keys) tree->remove(k);
        });
        for (auto k : keys) tree->insert(k, k);
        decltype(tree->split(N / 2)) split_result;
        double split_time = micro_duration([&] { split_result = tree->split(N / 2); });
        double merge_time = micro_duration([&] { tree->merge(std::move(split_result)); });
        if (include_sequential) {
            tree->clear();
            double seq_insert_time = duration([&] {
                for (int i = 0; i < N; i++) tree->insert(i, i);
            });
            double seq_find_time = duration([&] {
                for (int i = 0; i < N; i++) volatile auto _ = tree->find(i);
            });
            cout << format(
                "{:<10} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f} {:>16.2f} {:>16.2f}\n",
                name, insert_time, find_time, remove_time, split_time, merge_time, seq_insert_time,
                seq_find_time);
        } else {
            cout << format(
                "{:<10} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f} {:>16} {:>16}\n", name,
                insert_time, find_time, remove_time, split_time, merge_time, "N/A", "N/A");
        }
    };

    cout << format(
        "{:<10} {:>10} {:>10} {:>10} {:>10} {:>10} {:>16} {:>16}\n", "Tree", "Insert(ms)",
        "Find(ms)", "Remove(ms)", "Split(us)", "Merge(us)", "SeqInsert(ms)", "SeqFind(ms)");

    bench(make_unique<BasicTree<Key, Value>>(), "Basic", false);
    bench(make_unique<AVLTree<Key, Value>>(), "AVL");
    bench(make_unique<Treap<Key, Value>>(), "Treap");
    bench(make_unique<SplayTree<Key, Value>>(), "Splay");
}

struct BenchmarkMetric {
    size_t n;
    struct {
        std::chrono::duration<double> insert, find, remove, split, merge;
    } random, sequential;
};

enum class OperationType : std::uint8_t { INSERT, FIND, REMOVE, SPLIT, MERGE };

struct BenchmarkResult {
    std::string name;
    std::vector<BenchmarkMetric> data;
};

template <
    template <typename, typename> typename TreeType, template <typename, typename> typename... Args>
inline auto benchmarkData(const std::vector<int>& n_values) -> std::vector<BenchmarkResult> {
    std::vector<BenchmarkResult> results;
    if constexpr (sizeof...(Args)) results = benchmarkData<Args...>(n_values);
    auto duration = [](auto func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        return std::chrono::high_resolution_clock::now() - start;
    };
    auto bench = [&duration](auto& tree, auto& metrics, auto& keys) {
        tree->clear();
        metrics.insert = duration([&] {
            for (const auto& key : keys) tree->insert(key, key);
        });
        metrics.find = duration([&] {
            for (const auto& key : keys) volatile auto _ = tree->find(key);
        });
        metrics.remove = duration([&] {
            for (const auto& key : keys) tree->remove(key);
        });
        for (const auto& key : keys) tree->insert(key, key);
        std::unique_ptr<TreeType<int, int>> split_result;
        for (const auto& key : keys) {
            metrics.split += duration([&] { split_result = tree->split(key); });
            metrics.merge += duration([&] { tree->join(std::move(split_result)); });
        }
    };
    BenchmarkResult current;
    auto tree = make_unique<TreeType<int, int>>();
    current.name = tree->name();
    std::cout << std::format("current benchmark: {}\n", current.name);
    for (size_t n : n_values) {
        vector<size_t> keys(n);
        for (size_t i = 0; i < n; ++i) keys[i] = i;
        BenchmarkMetric metric{};  // NOTE: zero initialize!!!!!!!
        metric.n = n;
        // bench(tree, metric.sequential, keys);  TODO: visualize sequential data
        shuffle(keys.begin(), keys.end(), mt19937{random_device{}()});
        bench(tree, metric.random, keys);
        current.data.push_back(metric);
    }
    results.push_back(current);
    return results;
}

// 计算最小二乘法拟合的k值
template <typename F>
double fitK(const std::vector<double>& times, const std::vector<size_t>& ns, F f) {
    double sum_xy = 0, sum_xx = 0;
    for (size_t i = 0; i < times.size(); ++i) {
        double x = f(ns[i]);
        sum_xy += times[i] * x;
        sum_xx += x * x;
    }
    return sum_xy / sum_xx;
}

inline void visualizeBenchmarkData(const std::vector<BenchmarkResult>& data) {
    const int WIDTH = 1200, HEIGHT = 800, MARGIN = 120;
    const sf::Color COLORS[] = {
        sf::Color::Red, sf::Color::Green, sf::Color::Blue, sf::Color::Magenta, sf::Color::Cyan};
    const sf::Color THEORY_COLORS[] = {
        sf::Color(64, 64, 64, 192), sf::Color(144, 144, 144, 192)};  // 半透明灰色
    const std::string OPERATION_NAMES[] = {"Insert", "Find", "Remove", "Split", "Merge"};
    const OperationType OPERATIONS[] = {
        OperationType::INSERT, OperationType::FIND, OperationType::REMOVE, OperationType::SPLIT,
        OperationType::MERGE};

    // 为每个操作创建窗口
    std::vector<sf::RenderWindow> windows;
    for (const auto& op_name : OPERATION_NAMES) {
        windows.emplace_back(sf::VideoMode({WIDTH, HEIGHT}), "Benchmark: " + op_name);
    }

    sf::Font font;
    if (!font.openFromFile("assets/Menlo.ttf") && !font.openFromFile("../assets/Menlo.ttf") &&
        !font.openFromFile("../../assets/Menlo.ttf")) {
        throw std::runtime_error("Failed to load font");
    }

    // 计算每个操作的最大时间和平均时间
    std::vector<double> max_times(OPERATION_NAMES->size(), 0.0);
    std::vector<std::vector<double>> avg_times(OPERATION_NAMES->size());
    std::vector<size_t> ns;
    size_t max_n = 0;

    // 收集所有数据点
    for (const auto& metric : data[0].data) {
        ns.push_back(metric.n);
    }
    for (size_t i = 0; i < OPERATION_NAMES->size(); i++) avg_times[i].resize(ns.size());
    for (const auto& result : data) {
        size_t idx = 0;
        for (const auto& metric : result.data) {
            max_n = std::max(max_n, metric.n);
            max_times[0] = std::max(max_times[0], metric.random.insert.count());
            max_times[1] = std::max(max_times[1], metric.random.find.count());
            max_times[2] = std::max(max_times[2], metric.random.remove.count());
            max_times[3] = std::max(max_times[3], metric.random.split.count());
            max_times[4] = std::max(max_times[4], metric.random.merge.count());
            avg_times[0][idx] += metric.random.insert.count();
            avg_times[1][idx] += metric.random.find.count();
            avg_times[2][idx] += metric.random.remove.count();
            avg_times[3][idx] += metric.random.split.count();
            avg_times[4][idx] += metric.random.merge.count();
            idx++;
        }
    }

    // 计算平均值
    // debug(data.size());
    for (auto& times : avg_times) {
        for (auto& t : times) t /= data.size();
    }

    // 将所有时间转换为毫秒
    for (auto& t : max_times) t *= 1000;
    for (auto& times : avg_times) {
        for (auto& t : times) t *= 1000;
    }

    // 计算每个操作的k值
    std::vector<double> k_log_n(OPERATION_NAMES->size());
    std::vector<double> k_sqrt_n(OPERATION_NAMES->size());
    // debug(ns.size());
    for (size_t op = 0; op < OPERATION_NAMES->size(); ++op) {
        size_t max_n = ns.back();
        double max_time = avg_times[op].back();
        // debug(avg_times[op].size());
        // debug(max_n, max_time);
        k_log_n[op] = max_time / max_n / std::log2(max_n);
        k_log_n[op] = fitK(avg_times[op], ns, [](size_t n) { return n * std::log2(n); });
        k_sqrt_n[op] = fitK(avg_times[op], ns, [](size_t n) { return n * (n); });
    }

    while (std::any_of(windows.begin(), windows.end(), [](const auto& w) { return w.isOpen(); })) {
        for (size_t op_idx = 0; op_idx < windows.size(); ++op_idx) {
            auto& window = windows[op_idx];
            if (!window.isOpen()) continue;

            while (auto event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) window.close();
            }
            window.clear(sf::Color::White);

            // 坐标变换
            auto x_map = [&](size_t n) {
                return MARGIN + (WIDTH - 2 * MARGIN) * (double(n) / max_n);
            };
            auto y_map = [&](double t) {
                return HEIGHT - MARGIN - (HEIGHT - 2 * MARGIN) * (t / max_times[op_idx]);
            };

            // 绘制理论曲线
            const int POINTS = 1000;
            sf::VertexArray log_n_curve(sf::PrimitiveType::LineStrip, POINTS);
            sf::VertexArray sqrt_n_curve(sf::PrimitiveType::LineStrip, POINTS);
            for (int i = 0; i < POINTS; ++i) {
                double n = (double)max_n * (i + 1) / POINTS;
                double t_log_n = n * k_log_n[op_idx] * std::log2(n);
                double t_sqrt_n = n * k_sqrt_n[op_idx] * (n);
                log_n_curve[i].position = sf::Vector2f(x_map(n), y_map(t_log_n));
                log_n_curve[i].color = THEORY_COLORS[0];
                sqrt_n_curve[i].position = sf::Vector2f(x_map(n), y_map(t_sqrt_n));
                sqrt_n_curve[i].color = THEORY_COLORS[1];
            }
            window.draw(log_n_curve);
            window.draw(sqrt_n_curve);

            // 坐标轴
            sf::RectangleShape x_axis(sf::Vector2f(WIDTH - 2 * MARGIN, 2));
            x_axis.setPosition(sf::Vector2f(MARGIN, HEIGHT - MARGIN));
            x_axis.setFillColor(sf::Color::Black);
            window.draw(x_axis);
            sf::RectangleShape y_axis(sf::Vector2f(2, HEIGHT - 2 * MARGIN));
            y_axis.setPosition(sf::Vector2f(MARGIN, MARGIN));
            y_axis.setFillColor(sf::Color::Black);
            window.draw(y_axis);

            // 画每条线
            for (size_t algo = 0; algo < data.size(); ++algo) {
                const auto& result = data[algo];
                sf::VertexArray line(sf::PrimitiveType::LineStrip, result.data.size());
                for (size_t i = 0; i < result.data.size(); ++i) {
                    double t = 0;
                    switch (OPERATIONS[op_idx]) {
                        case OperationType::INSERT:
                            t = result.data[i].random.insert.count() * 1000;
                            break;
                        case OperationType::FIND:
                            t = result.data[i].random.find.count() * 1000;
                            break;
                        case OperationType::REMOVE:
                            t = result.data[i].random.remove.count() * 1000;
                            break;
                        case OperationType::SPLIT:
                            t = result.data[i].random.split.count() * 1000;
                            break;
                        case OperationType::MERGE:
                            t = result.data[i].random.merge.count() * 1000;
                            break;
                    }
                    line[i].position = sf::Vector2f(x_map(result.data[i].n), y_map(t));
                    line[i].color = COLORS[algo % (sizeof(COLORS) / sizeof(COLORS[0]))];
                }
                window.draw(line);

                // 图例
                sf::Text legend{font};
                legend.setString(result.name);
                legend.setCharacterSize(20);
                legend.setFillColor(COLORS[algo % (sizeof(COLORS) / sizeof(COLORS[0]))]);
                legend.setPosition(sf::Vector2f(MARGIN + 10, MARGIN + 30 * algo));
                window.draw(legend);
            }

            // 添加理论曲线图例
            sf::Text theory_legend1{font}, theory_legend2{font};
            theory_legend1.setString(std::format("n log(n) (k={:.2e})", k_log_n[op_idx]));
            theory_legend2.setString(std::format("n^2 (k={:.2e})", k_sqrt_n[op_idx]));
            theory_legend1.setCharacterSize(20);
            theory_legend2.setCharacterSize(20);
            theory_legend1.setFillColor(THEORY_COLORS[0]);
            theory_legend2.setFillColor(THEORY_COLORS[1]);
            theory_legend1.setPosition(sf::Vector2f(MARGIN + 10, MARGIN + 30 * (data.size() + 1)));
            theory_legend2.setPosition(sf::Vector2f(MARGIN + 10, MARGIN + 30 * (data.size() + 2)));
            window.draw(theory_legend1);
            window.draw(theory_legend2);

            // Y轴最大值标签
            sf::Text y_label{font};
            y_label.setString(std::to_string(int(max_times[op_idx])) + " ms");
            y_label.setCharacterSize(16);
            y_label.setFillColor(sf::Color::Black);
            y_label.setPosition(sf::Vector2f(10, MARGIN - 10));
            window.draw(y_label);

            // X轴最大值标签
            sf::Text x_label{font};
            x_label.setString("n=" + std::to_string(max_n));
            x_label.setCharacterSize(16);
            x_label.setFillColor(sf::Color::Black);
            x_label.setPosition(sf::Vector2f(WIDTH - MARGIN - 40, HEIGHT - MARGIN + 10));
            window.draw(x_label);

            window.display();
        }
    }
}

inline void benchmark() {
    crtpBenchmark();
    algorithmBenchmark();
    std::cout << std::format("\n===== Visualize =====\n");
    std::vector<int> n_values;
    for (int i = 1; i <= 40; i++) {
        n_values.push_back(i * 25000);
    }
    auto data = benchmarkData<AVLTree, Treap, SplayTree>(n_values);
    std::cout << "done\n";
    visualizeBenchmarkData(data);
}
