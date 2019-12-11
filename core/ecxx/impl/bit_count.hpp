#pragma once

namespace ecxx {

constexpr unsigned bit_count(const unsigned long long x) noexcept {
    return (x == 0u) ? 0u : ((x & 1u) + bit_count(x >> 1u));
}

}