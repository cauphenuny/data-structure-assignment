/// @file events.hpp
/// @brief GUI events and handlers

#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <optional>

// 事件类型枚举
enum class EventType : uint8_t{
    WINDOW_CLOSE,
    WINDOW_RESIZE,
    MOUSE_WHEEL,
    MOUSE_PRESS,
    MOUSE_RELEASE,
    MOUSE_MOVE,
};

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

template<>
struct EventTypeMap<sf::Event::MouseWheelScrolled> {
    static constexpr EventType value = EventType::MOUSE_WHEEL;
};

template<>
struct EventTypeMap<sf::Event::MouseButtonPressed> {
    static constexpr EventType value = EventType::MOUSE_PRESS;
};

template<>
struct EventTypeMap<sf::Event::MouseButtonReleased> {
    static constexpr EventType value = EventType::MOUSE_RELEASE;
};

template<>
struct EventTypeMap<sf::Event::MouseMoved> {
    static constexpr EventType value = EventType::MOUSE_MOVE;
};

template<typename TEventSubtype>
constexpr EventType event_t = EventTypeMap<TEventSubtype>::value;

// 类型别名
using Event = std::optional<sf::Event>;
using EventListener = std::function<void(const Event&)>;
using ListenerMap = std::multimap<EventType, EventListener>;
using ListenerHandle = ListenerMap::iterator;

class EventTarget {
public:
    template<typename TEventSubtype>
    ListenerHandle on(EventListener callback) {
        // auto wrapper = [callback](const Event& event) {
        //     if (!event || !event->is<TEventSubtype>())
        //         return;
        //     const auto& typedEvent = event->getIf<TEventSubtype>();
        //     if (!typedEvent) return;
        //     callback(typedEvent);
        // };
        return listeners_.insert({event_t<TEventSubtype>, callback});
    }
    
    template<typename... TEventSubtypes>
    inline void dispatchEvent(const Event& event) {
        if (!event) return;
        (dispatchIfType<TEventSubtypes>(event), ...);
    }

private:
    ListenerMap listeners_;

    template<typename TEventSubtype>
    inline void dispatchIfType(const Event& event) {
        if (!event->is<TEventSubtype>()) return;
        auto type = event_t<TEventSubtype>;
        auto range = listeners_.equal_range(type);
        for (auto it = range.first; it != range.second; ++it) {
            it->second(event);
        }
    }

};
