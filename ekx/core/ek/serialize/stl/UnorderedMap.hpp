#pragma once

#include "../core.hpp"

#include <unordered_map>

namespace ek {

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

}