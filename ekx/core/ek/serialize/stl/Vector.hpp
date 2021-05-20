#pragma once

#include "../core.hpp"

#include <vector>

namespace ek {

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

}