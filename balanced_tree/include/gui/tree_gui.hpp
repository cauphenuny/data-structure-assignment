#pragma once
#include "gui/balanced_tree_renderer.hpp"
#include "gui/gui.hpp"
#include "tree/avl.hpp"
#include "tree/basic.hpp"
#include "tree/interface.hpp"
#include "tree/splay.hpp"
#include "tree/treap.hpp"

#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>

// 树类型工厂模板
template <typename K, typename V> class TreeFactory {
public:
    template <template <typename, typename> class TreeImpl>
    static std::unique_ptr<Tree<K, V>> create() {
        return TreeAdapter<K, V, TreeImpl>::create();
    }

    // 获取树类型名称
    template <template <typename, typename> class TreeImpl> static std::string getName() {
        auto tree = create<TreeImpl>();
        return tree->name();
    }
};

// 树GUI管理器
template <typename K = int, typename V = std::string> class TreeGUI : public GUIBase {
public:
    using TreePtr = std::unique_ptr<Tree<K, V>>;
    using TreeFactoryFunc = std::function<TreePtr()>;

    TreeGUI()
        : GUIBase(), balancedTreeRenderer(font), currentTraceStep(0), isPlayingTrace(false),
          tracePlaySpeed(1.0f), isInputActive(false), treeTypeText(font), treeInfoText(font) {

        // 注册不同的树类型
        registerTreeType<AVLTreeImpl>(
            "AVL Tree", []() { return TreeFactory<K, V>::template create<AVLTreeImpl>(); });
        registerTreeType<SplayTreeImpl>(
            "Splay Tree", []() { return TreeFactory<K, V>::template create<SplayTreeImpl>(); });
        registerTreeType<TreapImpl>(
            "Treap", []() { return TreeFactory<K, V>::template create<TreapImpl>(); });
        registerTreeType<BasicTreeImpl>(
            "Basic BST", []() { return TreeFactory<K, V>::template create<BasicTreeImpl>(); });

        // 默认使用AVL树
        switchToTree(0);
        setupEventHandlers();
    }

    virtual ~TreeGUI() = default;

    // 树操作接口
    void insertKey(const K& key, const V& value = V{}) {
        if (!currentTree) {
            std::cout << "oh no\n";
            return;
        }
        std::cout << "oh yeah\n";

        try {
            // 执行操作并捕获trace
            currentTrace =
                currentTree->trace([this, key, value]() { currentTree->insert(key, value); });

            // 如果有动画步骤，开始播放
            if (currentTrace.size() > 1) {
                currentTraceStep = 0;
                playTrace();
            } else {
                std::cout << "no trace!\n";
                // 没有trace，直接更新显示
                updateTreeDisplay();
            }
        } catch (const std::exception& e) {
            std::cerr << "Insert error: " << e.what() << std::endl;
            updateTreeDisplay();  // 至少更新显示
        }
    }

    void removeKey(const K& key) {
        if (!currentTree) return;

        try {
            currentTrace = currentTree->trace([this, key]() { currentTree->remove(key); });

            if (currentTrace.size() > 1) {
                currentTraceStep = 0;
                playTrace();
            } else {
                updateTreeDisplay();
            }
        } catch (const std::exception& e) {
            std::cerr << "Remove error: " << e.what() << std::endl;
            updateTreeDisplay();
        }
    }

    void findKey(const K& key) {
        if (!currentTree) return;

        try {
            // 查找操作可能有访问路径的trace
            currentTrace = currentTree->trace([this, key]() { currentTree->find(key); });

            if (currentTrace.size() > 1) {
                currentTraceStep = 0;
                playTrace();
            }
        } catch (const std::exception& e) {
            std::cerr << "Find error: " << e.what() << std::endl;
        }
    }

    void clearTree() {
        if (!currentTree) return;
        currentTree->clear();
        currentTrace.clear();
        currentTraceStep = 0;
        updateTreeDisplay();
    }

    // 切换树类型
    void switchToTree(size_t index) {
        if (index >= treeTypes.size()) return;

        try {
            // 保存当前数据
            std::vector<std::pair<K, V>> treeData;
            if (currentTree) {
                currentTree->traverse(
                    [&treeData](const K& key, V& value) { treeData.emplace_back(key, value); });
            }

            // 创建新树
            currentTreeIndex = index;
            currentTree = treeTypes[index].factory();

            // 重新插入数据
            for (const auto& [key, value] : treeData) {
                currentTree->insert(key, value);
            }

            updateTreeDisplay();
            updateUITexts();
        } catch (const std::exception& e) {
            std::cerr << "Switch tree error: " << e.what() << std::endl;
        }
    }

    // 动画控制
    void playTrace() {
        if (!currentTrace.empty()) {
            std::cout << "play trace!\n";
            isPlayingTrace = true;
            traceClock.restart();
        }
    }

    void pauseTrace() { isPlayingTrace = false; }

    void resetTrace() {
        currentTraceStep = 0;
        isPlayingTrace = false;
        if (!currentTrace.empty()) {
            balancedTreeRenderer.updateFromTrace(currentTrace, currentTraceStep);
        }
    }

    void stepTrace(bool forward = true) {
        if (currentTrace.empty()) return;

        size_t newStep = currentTraceStep;
        if (forward && currentTraceStep < currentTrace.size() - 1) {
            newStep = currentTraceStep + 1;
        } else if (!forward && currentTraceStep > 0) {
            newStep = currentTraceStep - 1;
        }

        if (newStep != currentTraceStep) {
            // 使用动画过渡
            if (newStep < currentTrace.size() && currentTraceStep < currentTrace.size()) {
                balancedTreeRenderer.animateTransition(
                    currentTrace[currentTraceStep], currentTrace[newStep]);
            }
            currentTraceStep = newStep;
        }
    }

protected:
    void initUI() override {
        // 调用基类的初始化
        GUIBase::initUI();
        setupUI();
    }

    void initTreeData() override {
        // 插入示例数据
        try {
            if constexpr (std::is_same_v<K, int>) {
                insertKey(K{10}, getValue("10"));
                insertKey(K{5}, getValue("5"));
                insertKey(K{15}, getValue("15"));
                insertKey(K{3}, getValue("3"));
                insertKey(K{7}, getValue("7"));
            }
        } catch (const std::exception& e) {
            std::cerr << "Init tree data error: " << e.what() << std::endl;
        }
    }

    void render() override { GUIBase::render(); }

    void update(float deltaTime) override {
        balancedTreeRenderer.update(deltaTime);
        updateTraceAnimation(deltaTime);
        updateUITexts();
    }

private:
    // 树类型信息
    struct TreeTypeInfo {
        std::string name;
        TreeFactoryFunc factory;
    };

    std::vector<TreeTypeInfo> treeTypes;
    size_t currentTreeIndex = 0;
    TreePtr currentTree;

    // 渲染器
    BalancedTreeRenderer balancedTreeRenderer;

    // 动画和trace
    std::vector<ForestView> currentTrace;
    size_t currentTraceStep;
    bool isPlayingTrace;
    float tracePlaySpeed;
    sf::Clock traceClock;

    // UI元素
    sf::RectangleShape controlPanel;
    sf::Text treeTypeText;
    sf::Text treeInfoText;
    std::vector<sf::Text> controlTexts;

    // 输入
    std::string inputBuffer;
    bool isInputActive;

    // 获取值的辅助函数
    V getValue(const std::string& str) const {
        if constexpr (std::is_same_v<V, std::string>) {
            return str;
        } else if constexpr (std::is_same_v<V, int>) {
            return std::stoi(str);
        } else {
            return V{};
        }
    }

    // 注册树类型
    template <template <typename, typename> class TreeImpl>
    void registerTreeType(const std::string& name, TreeFactoryFunc factory) {
        treeTypes.push_back({name, factory});
    }

    void setupUI() {
        // 设置控制面板
        controlPanel.setSize({300, static_cast<float>(windowSize.y)});
        controlPanel.setPosition({windowSize.x - 300, 0});
        controlPanel.setFillColor(sf::Color(240, 240, 240, 200));

        // 初始化文本
        treeTypeText.setFont(font);
        treeTypeText.setCharacterSize(18);
        treeTypeText.setFillColor(textColor);
        treeTypeText.setPosition({windowSize.x - 280, 20});

        treeInfoText.setFont(font);
        treeInfoText.setCharacterSize(14);
        treeInfoText.setFillColor(textColor);
        treeInfoText.setPosition({windowSize.x - 280, 60});

        // 设置渲染器
        splitView.setLeftViewRenderer([this](sf::RenderWindow& window, const sf::View& view) {
            balancedTreeRenderer.render(window);
        });

        splitView.setRightViewRenderer([this](sf::RenderWindow& window, const sf::View& view) {
            window.draw(controlPanel);
            window.draw(treeTypeText);
            window.draw(treeInfoText);

            for (const auto& text : controlTexts) {
                window.draw(text);
            }
        });

        updateUITexts();
    }

    void setupEventHandlers() {
        // 添加键盘事件到基类的事件处理中
        on<sf::Event::KeyPressed>([this](const Event& event) {
            const auto& keyEvent = event->getIf<sf::Event::KeyPressed>();

            try {
                switch (keyEvent->code) {
                    case sf::Keyboard::Key::Num1: switchToTree(0); break;
                    case sf::Keyboard::Key::Num2:
                        if (treeTypes.size() > 1) switchToTree(1);
                        break;
                    case sf::Keyboard::Key::Num3:
                        if (treeTypes.size() > 2) switchToTree(2);
                        break;
                    case sf::Keyboard::Key::Num4:
                        if (treeTypes.size() > 3) switchToTree(3);
                        break;
                    case sf::Keyboard::Key::Space:
                        if (isPlayingTrace)
                            pauseTrace();
                        else
                            playTrace();
                        break;
                    case sf::Keyboard::Key::Left: stepTrace(false); break;
                    case sf::Keyboard::Key::Right: stepTrace(true); break;
                    case sf::Keyboard::Key::R: resetTrace(); break;
                    case sf::Keyboard::Key::C: clearTree(); break;
                    case sf::Keyboard::Key::Enter:
                        if (!inputBuffer.empty()) {
                            executeCommand(inputBuffer);
                            inputBuffer.clear();
                        }
                        break;
                    case sf::Keyboard::Key::Backspace:
                        if (!inputBuffer.empty()) {
                            inputBuffer.pop_back();
                        }
                        break;
                    default: break;
                }
                updateUITexts();
            } catch (const std::exception& e) {
                std::cerr << "Key event error: " << e.what() << std::endl;
            }
        });

        // 文本输入
        on<sf::Event::TextEntered>([this](const Event& event) {
            const auto& textEvent = event->getIf<sf::Event::TextEntered>();
            handleTextInput(textEvent->unicode);
        });
    }

    void handleTextInput(uint32_t unicode) {
        if (unicode >= 32 && unicode < 127) {
            char c = static_cast<char>(unicode);
            if (std::isdigit(c) || c == ' ' || c == '-' || std::isalpha(c)) {
                inputBuffer += c;
                updateUITexts();
            }
        }
    }

    void executeCommand(const std::string& command) {
        std::istringstream iss(command);
        std::string cmd;
        if (!(iss >> cmd)) return;

        try {
            if constexpr (std::is_same_v<K, int>) {
                if (cmd == "insert" || cmd == "i") {
                    int key;
                    if (iss >> key) {
                        if constexpr (std::is_same_v<V, std::string>) {
                            std::string value;
                            if (iss >> value) {
                                insertKey(key, value);
                            } else {
                                insertKey(key, std::to_string(key));
                            }
                        } else {
                            insertKey(key);
                        }
                    }
                } else if (cmd == "remove" || cmd == "r") {
                    int key;
                    if (iss >> key) {
                        removeKey(key);
                    }
                } else if (cmd == "find" || cmd == "f") {
                    int key;
                    if (iss >> key) {
                        findKey(key);
                    }
                } else {
                    // 尝试直接解析为数字
                    try {
                        int key = std::stoi(command);
                        if constexpr (std::is_same_v<V, std::string>) {
                            insertKey(key, std::to_string(key));
                        } else {
                            insertKey(key);
                        }
                    } catch (...) {
                        // 忽略无效输入
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Command execution error: " << e.what() << std::endl;
        }
    }

    void updateTreeDisplay() {
        try {
            if (currentTree) {
                balancedTreeRenderer.updateFromTree(currentTree.get());
            } else {
                balancedTreeRenderer.clear();
            }
        } catch (const std::exception& e) {
            std::cerr << "Update tree display error: " << e.what() << std::endl;
        }
    }

    void updateTraceAnimation(float deltaTime) {
        if (!isPlayingTrace || currentTrace.empty()) return;

        std::cout << "update trace!\n";

        // 等待当前动画完成
        if (balancedTreeRenderer.hasActiveAnimations()) return;

        std::cout << "end waiting trace!\n";

        float elapsed = traceClock.getElapsedTime().asSeconds();
        if (elapsed >= 1.0f / tracePlaySpeed) {
            stepTrace(true);
            traceClock.restart();

            if (currentTraceStep >= currentTrace.size() - 1) {
                isPlayingTrace = false;
            }
        }
    }

    void updateUITexts() {
        try {
            // 更新树类型显示
            if (currentTreeIndex < treeTypes.size()) {
                treeTypeText.setString("Current: " + treeTypes[currentTreeIndex].name);
            }

            // 更新树信息
            std::ostringstream oss;
            if (currentTree) {
                oss << "Size: " << currentTree->size() << " nodes\n";
                if (currentTrace.size() > 1) {
                    oss << "Animation: " << (currentTraceStep + 1) << "/" << currentTrace.size();
                    if (isPlayingTrace) oss << " (Playing)";
                }
            }
            treeInfoText.setString(oss.str());

            // 更新控制说明
            controlTexts.clear();
            std::vector<std::string> instructions = {
                "Controls:",
                "1-4: Switch tree type",
                "Enter: Execute command",
                "Space: Play/Pause animation",
                "Left/Right: Step animation",
                "R: Reset animation",
                "C: Clear tree",
                "",
                "Commands:",
                "insert <key> [value]",
                "remove <key>",
                "find <key>",
                "Or just type a number",
                "",
                "Input: " + inputBuffer};

            float yPos = 120;
            for (const auto& instruction : instructions) {
                sf::Text text(font, instruction, 12);
                text.setFillColor(textColor);
                text.setPosition({windowSize.x - 280, yPos});
                controlTexts.push_back(text);
                yPos += 16;
            }
        } catch (const std::exception& e) {
            std::cerr << "Update UI texts error: " << e.what() << std::endl;
        }
    }
};

// 便利的类型别名
// using IntStringTreeGUI = TreeGUI<int, std::string>;
// using IntTreeGUI = TreeGUI<int, int>;
