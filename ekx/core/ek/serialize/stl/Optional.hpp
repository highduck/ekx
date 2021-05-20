#pragma once

#include "../core.hpp"

#include <optional>

namespace ek {

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

}