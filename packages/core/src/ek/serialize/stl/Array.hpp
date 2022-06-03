#pragma once

#include "../core.hpp"

#include <array>

namespace ek {

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

}
