#pragma once

#include "core.hpp"

#include <ek/ds/Array.hpp>
#include <ek/ds/StaticArray.hpp>
#include <ek/ds/FixedArray.hpp>

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

}