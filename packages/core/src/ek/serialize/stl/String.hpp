#pragma once

#include "../core.hpp"
#include <string>

namespace ek {

template<typename S>
inline void serialize(IO<S>& io, std::string& value) {
    if constexpr (is_readable_stream<S>()) {
        int32_t size;
        io.value(size);
        value.resize(size);
        io.span(value.data(), size);

        // null-terminator
        uint8_t term;
        io.value(term);
        EK_ASSERT(term == 0);
    } else {
        auto size = static_cast<int32_t>(value.size());
        io.value(size);
        io.span(value.data(), size);

        // null-terminator
        const uint8_t term = 0;
        io.value(term);
    }
}

}
