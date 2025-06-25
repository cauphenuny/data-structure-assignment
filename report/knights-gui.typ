== 图形用户界面

图形用户界面的实现基于第三方库 #link("https://github.com/SFML/SFML")[SFML]（Simple and Fast Multimedia Library），这是一个基于 C++ 开发的开源多媒体库，其一大特点是跨平台支持。

#figure(image("assets/sfml.png", width: 70%), caption: [SFML 官网])

我们使用的是SFML的最新版本3.0.1。

SFML在从版本2升级到3时做了巨大调整，虽然代码设计得到优化，并开始支持更多高级特性，但这也导致SFML 3的API与SFML 2的基本完全不兼容。而现有的绝大部分资料都是关于SFML 2的使用（距离此次重大版本更新仅仅过去半年，而SFML 2已存在12余年），甚至SFML 3的官方文档也尚不完善。

#grid(
  columns: (0.45fr, 0.6fr),
  [
    在查看API接口源代码以外，我们还使用了一个强大的工具——#link("https://deepwiki.org/")[DeepWiki]。它可以根据 GitHub 仓库的文件内容，生成仓库专属Wiki，包括仓库整体架构、主要功能模块和实现方式等。它还支持用户询问仓库相关信息。这一工具极大方便我们快速了解仓库代码功能、仓库结构等重要信息，节省大量时间。
  ], [
    #figure(image("assets/deepwiki.png", width: 70%), caption: [SFML项目的DeepWiki页面])
  ]
)

---

=== GUI 设计

界面设计遵循关注点分离的原则，将棋盘渲染、控制面板、动画管理、事件分发与处理等功能封装到不同的类中。类与类之间维持继承与组合关系，形成下面的关系网。

#figure(image("assets/knights-gui.png", width: 75%), caption: [GUI 结构])

界面的主逻辑采用经典的游戏循环架构，即最外层循环判断窗口是否开启，循环内部进行事件处理、逻辑更新和渲染绘制三个核心阶段。

```cpp
void KnightsTourGUI::run() {
    while (window.isOpen()) {
        static sf::Clock deltaClock;
        float deltaTime = deltaClock.restart().asSeconds();

        // 检查事件
        while (const std::optional event = window.pollEvent()) {        
            dispatchEvent<sf::Event::Closed,
                          sf::Event::Resized,
                          sf::Event::MouseButtonPressed,
                          sf::Event::KeyPressed>
                (event);
        }

        update(deltaTime);
        render();
    }
}
```

---

事件处理方面，使用模板特化与折叠表达式，实现诸多事件的统一分发。

#grid(
  columns: (0.6fr, 0.45fr),
  [
    ```cpp
    // 事件类型枚举
    enum class EventType : uint8_t{
        WINDOW_CLOSE,
        WINDOW_RESIZE,
        MOUSE_WHEEL,
        ...
    };

    class EventTarget {
    public:
        template<typename... TEventSubtypes>
        inline void dispatchEvent(const Event& event) {
            if (!event) return;
            (dispatchIfType<TEventSubtypes>(event), ...);
        }
    private:
        ListenerMap listeners_;

        template<typename TEventSubtype>
        inline void dispatchIfType(const Event& event) {...}
    };

    ```
  ],[

    ```cpp
    // 通过特化实现 SFML 事件类型到 EventType 的映射
    template<typename TEventSubtype>
    struct EventTypeMap;

    template<>
    struct EventTypeMap<sf::Event::Closed> {
        static constexpr EventType value = EventType::WINDOW_CLOSE;
    };

    template<>
    struct EventTypeMap<sf::Event::Resized> {
        static constexpr EventType value = EventType::WINDOW_RESIZE;
    };

    ...

    template<typename TEventSubtype>
    constexpr EventType event_t = EventTypeMap<TEventSubtype>::value;
    ```

  ]
)

这样写看上去很复杂。实际上在SFML 2中实现同样逻辑只需寥寥数行代码，见左边代码块。这是因为SFML 2中的`sf::Event event`包含一个可以直接访问的成员`enum EventType type`，可以直接获取事件的类型并进行比较；而SFML 3中则删除了`type`，取而代之的是一个私有成员`m_data`。两个版本的`Event`类的结构如右边代码块所示（注释内是SFML 2的版本）。

可见SFML 3使用`std::variant`实现了更安全的事件类，但代价是对事件的处理也更加复杂。

#grid(
  columns: (0.4fr, 0.5fr),
  [
    ```cpp
    switch (event.type) {
        case sf::Event::Closed:
            window.close();
            break;

        case sf::Event::Resized:
            ...
            break;
        ...

        default:
            break;
    }
    ```
  ],
  [
    ```cpp
    namespace sf {
        class Event {
        public:
            struct Closed {};
            struct Resized { Vector2u size; }
            ...
        /*
            enum EventType {Closed, Resized, ...};
            EventType type;
        */
        private:
            std::variant<Closed, Resized, ...> m_data;
        }
    }
    ```
  ]
)

---

=== GUI 展示

界面分为左右两栏，左侧为棋盘显示区域，右侧为控制面板。用户可以指定马的初始位置，选择执行的算法，调整播放速度等；棋盘区域将展示马的移动轨迹。

#figure(image("assets/knights-gui-demo.png", width: 53%), caption: [GUI 演示])

经过实测，此图形用户界面在小组成员的不同开发环境中均能正常工作。