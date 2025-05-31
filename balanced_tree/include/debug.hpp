/// @file debug.hpp
/// @brief utility functions for debug serialization

#pragma once
#include <filesystem>
#include <iostream>
#include <list>
#include <map>
#include <source_location>
#include <sstream>
#include <string>

inline std::string addIndent(std::string_view str, int indent = 1) {
    std::string indent_str;
    for (int i = 0; i < indent; i++) indent_str += "    ";
    bool indent_flag = 1;
    std::string result_str;
    for (auto i : str) {
        if (indent_flag) result_str += indent_str, indent_flag = 0;
        result_str += i;
        if (i == '\n') {
            indent_flag = 1;
        }
    }
    if (result_str.back() != '\n') result_str += '\n';
    return result_str;
}

inline std::string compressString(std::string_view str) {
    std::string ret;
    char prev = ' ';
    for (auto i : str) {
        if (!isspace(i))
            ret += i;
        else if (!isspace(prev))
            ret += ' ';
        prev = i;
    }
    if (str.back() == '\n') ret += '\n';
    return ret;
}

inline std::string tryCompressString(const std::string& str) {
    if (str.length() < 80)
        return compressString(str);
    else
        return std::string(str);
}

template <std::input_iterator T> std::string stringify(const T& begin, const T& end) {
    std::string str;
    size_t index = 0;
    for (T it = begin; it != end; it++) {
        str += std::format("[{}]: {},\n", index++, serialize(*it));
    }
    if (!str.empty()) {
        str.pop_back(), str.pop_back();
    }
    return tryCompressString(str);
}

template <typename T1, typename T2> std::string stringify(const std::pair<T1, T2>& p) {
    return std::format("{}: {}", serialize(p.first), serialize(p.second));
}

template <typename T> std::string stringify(const std::vector<T>& vec) {
    return tryCompressString(
        "std::vector {\n" + addIndent(stringify(vec.begin(), vec.end())) + "}");
}

template <typename K, typename V> std::string stringify(const std::map<K, V>& m) {
    return tryCompressString("std::map {\n" + addIndent(stringify(m.begin(), m.end())) + "}");
}

template <typename T> std::string stringify(const std::list<T>& l) {
    return tryCompressString("std::list {\n" + addIndent(stringify(l.begin(), l.end())) + "}");
}

template <typename T> std::string serialize(const std::unique_ptr<T>& ptr) {
    if constexpr (requires { ptr->stringify(); }) {
        return ptr ? ptr->stringify() : "nullptr";
    } else {
        return serialize(*ptr);
    }
}

std::string serialize(const auto& val) {
    if constexpr (requires { std::string(val); }) {
        return "\"" + std::string(val) + "\"";
    } else if constexpr (requires { stringify(val); }) {
        return stringify(val);
    } else if constexpr (requires { val.stringify(); }) {
        return val.stringify();
    } else if constexpr (requires { std::to_string(val); }) {
        return std::to_string(val);
    } else if constexpr (requires { std::ostringstream() << val; }) {
        static std::ostringstream oss;
        oss.str(""), oss.clear(), oss << val;
        return oss.str();
    } else {
        static_assert(false, "can not convert to string");
    }
}

std::string serializeVar(const char* names, const auto& var, const auto&... rest) {
    std::ostringstream oss;
    const char* comma = strchr(names, ',');
    while (names[0] == ' ') names++;
    if (comma != nullptr) {
        oss.write(names, comma - names) << ": " << serialize(var) << ","
                                        << "\n";
        if constexpr (sizeof...(rest)) oss << serializeVar(comma + 1, rest...);
    } else {
        oss.write(names, strlen(names)) << ": " << serialize(var) << "\n";
    }
    return oss.str();
}

#define serializeClass(name, ...) \
    tryCompressString(name " {\n" + addIndent(serializeVar(#__VA_ARGS__, __VA_ARGS__)) + "}")

template <typename T>
    requires(
        requires(T t) { t.stringify(); } || requires(T t) { stringify(t); })
struct std::formatter<T> : std::formatter<std::string> {
    auto format(const T& t, std::format_context& ctx) const {
        std::string str;
        if constexpr (requires { t.stringify(); })
            str = t.stringify();
        else
            str = stringify(t);
        return std::formatter<std::string>::format(str, ctx);
    }
};

template <typename T>
    requires requires(T t) { t.stringify(); } || requires(T t) { stringify(t); }
std::ostream& operator<<(std::ostream& os, const T& t) {
    if constexpr (requires { t.stringify(); })
        os << t.stringify();
    else
        os << stringify(t);
    return os;
}

inline std::string getLocation(std::source_location location = std::source_location::current()) {
    return std::format(
        "[[ {}:{} in `{}` ]]", std::filesystem::path(location.file_name()).filename().string(),
        location.line(), location.function_name());
}

#define debug(...) std::cerr << getLocation() << "\n" << serializeVar(#__VA_ARGS__, __VA_ARGS__)
