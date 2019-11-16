#pragma once

#include "core.hpp"

#include <cstring>
#include <string>
#include <vector>
#include <array>
#include <optional>
#include <map>
#include <unordered_map>

namespace ek {

/** specializations **/

template<typename S>
inline void serialize(IO<S>& io, std::string& value) {
    if constexpr (is_readable_stream<S>()) {
        uint32_t size;
        io.value(size);
        value.resize(size);
        io.span(value.data(), size);
    } else {
        auto size = static_cast<uint32_t>(value.size());
        io.value(size);
        io.span(value.data(), size);
    }
}

template<typename S>
inline void serialize(IO<S>& io, bool& value) {
    if constexpr (is_readable_stream<S>()) {
        uint8_t v;
        io.value(v);
        value = v != 0;
    } else {
        uint8_t v = value ? 1u : 0u;
        io.value(v);
    }
}

template<typename S, typename T>
inline void serialize(IO<S>& io, std::vector<T>& value) {
    uint32_t size;

    if constexpr (is_readable_stream<S>()) {
        io.value(size);
        value.resize(size);
    } else {
        size = static_cast<uint32_t>(value.size());
        io.value(size);
    }

    if constexpr(is_pod_type<T>()) {
        io.span(value.data(), size * sizeof(T));
    } else {
        for (T& el : value) {
            io(el);
        }
    }
}

template<typename S, typename K, typename V>
inline void serialize(IO<S>& io, std::unordered_map<K, V>& value) {
    uint32_t size;

    if constexpr (is_readable_stream<S>()) {
        io.value(size);
        value.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            K k; V v;
            io(k, v);
            value[k] = v;
        }
    } else {
        size = static_cast<uint32_t>(value.size());
        io.value(size);
        for (auto& pair : value) {
            io(const_cast<K&>(pair.first), pair.second);
        }
    }
}

template<typename S, typename K, typename V>
inline void serialize(IO<S>& io, std::map<K, V>& value) {
    uint32_t size;

    if constexpr (is_readable_stream<S>()) {
        io.value(size);
        value.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            K k; V v;
            io(k, v);
            value[k] = v;
        }
    } else {
        size = static_cast<uint32_t>(value.size());
        io.value(size);
        for (auto& pair : value) {
            io(const_cast<K&>(pair.first), pair.second);
        }
    }
}

template<typename S, typename T, size_t Size>
inline void serialize(IO<S>& io, std::array<T, Size>& value) {
    if constexpr(is_pod_type<T>()) {
        io.span(value.data(), Size * sizeof(T));
    } else {
        for (T& el : value) {
            io(el);
        }
    }
}

//#  if  __cplusplus <= 201103L
//#warning "<=201103L"
//#  elif __cplusplus <= 201402L
//#warning "14"
//#  elif __cplusplus <= 201703L
//#warning "17"
//#  else
//#warning "18"
//#  endif

// _LIBCPP_STD_VER > 14
#if __cplusplus > 201402L

template<typename S, typename T>
inline void serialize(IO<S>& io, std::optional<T>& value) {
    uint8_t has;

    if constexpr (is_readable_stream<S>()) {
        io.value(has);
        if (has != 0) {
            value.emplace();
        }
    } else {
        has = value.has_value() ? 1u : 0u;
        io.value(has);
    }

    if (has != 0) {
        io(*value);
    }
}

#endif
}