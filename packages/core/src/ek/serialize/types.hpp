#pragma once

#include "core.hpp"

#include <ek/ds/Array.hpp>
#include <ek/ds/Hash.hpp>
#include <ek/ds/String.hpp>

namespace ek {

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
inline void serialize(IO<S>& io, Array<T>& value) {
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

template<typename S, typename V>
inline void serialize(IO<S>& io, Hash<V>& value) {
    uint32_t size;

    if constexpr (is_readable_stream<S>()) {
        io.value(size);
        value.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            uint64_t k; V v;
            io(k, v);
            value.set(k, v);
        }
    } else {
        size = static_cast<uint32_t>(value.size());
        io.value(size);
        for (const auto& entry : value) {
            io(entry.key, entry.value);
        }
    }
}

template<typename S>
inline void serialize(IO<S>& io, String& value) {
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

}