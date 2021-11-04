#pragma once

#include "../core.hpp"
#include <string>

namespace ek {

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

}