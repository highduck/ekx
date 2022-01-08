#pragma once

#include <cstdint>
#include <ek/math.h>

namespace ek {

inline uint8_t sat_add_u8(uint8_t a, uint8_t b) {
    uint8_t c;
    return __builtin_add_overflow(a, b, &c) ? 0xFF : c;
}

struct argb32_t final {

    static const argb32_t zero;
    static const argb32_t one;
    static const argb32_t black;

    union {
        struct {
            uint8_t b, g, r, a;
        };
        uint32_t argb;
    };

    argb32_t() noexcept {

    }
//    constexpr argb32_t(const argb32_t&) noexcept = default;
//    constexpr argb32_t(argb32_t&&) noexcept = default;

    constexpr argb32_t(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_) noexcept
            : b{b_}, g{g_}, r{r_}, a{a_} {}

//        template<typename T>
    constexpr argb32_t(float r_, float g_, float b_, float a_ = 1.0f) noexcept
            : b{static_cast<uint8_t>(int(b_ * 255) & 0xFFu)},
              g{static_cast<uint8_t>(int(g_ * 255) & 0xFFu)},
              r{static_cast<uint8_t>(int(r_ * 255) & 0xFFu)},
              a{static_cast<uint8_t>(int(a_ * 255) & 0xFFu)} {
    }

    constexpr explicit argb32_t(const float v[4]) :
            argb32_t(v[0], v[1], v[2], v[3]) {
    }

//    template<typename T>
//    constexpr explicit argb32_t(const Vec<4, T>& vector)
//            :argb32_t(vector.x, vector.y, vector.z, vector.w) {
//    }

//    template<typename T>
//    constexpr explicit argb32_t(const Vec<3, T>& vector, float alpha = 1.0f)
//            :argb32_t(vector.x, vector.y, vector.z, alpha) {
//    }

    constexpr explicit argb32_t(uint32_t argb_) noexcept
            : argb{argb_} {}

    constexpr argb32_t(uint32_t rgb_, float alpha) :
            argb{rgb_ & 0xFFFFFFu} {
        af(alpha);
    }

    constexpr operator rgba_t() const noexcept {
//        return {{r, g, b, a}};
        return {.value = COL32_SWAP_RB(argb)};
    }

    constexpr argb32_t operator*(argb32_t multiplier) const {
        return argb32_t{
                uint8_t((r * multiplier.r * 258u) >> 16u),
                uint8_t((g * multiplier.g * 258u) >> 16u),
                uint8_t((b * multiplier.b * 258u) >> 16u),
                uint8_t((a * multiplier.a * 258u) >> 16u)
        };
    }

    argb32_t& operator*=(const argb32_t multiplier) {
        *this = *this * multiplier;
        return *this;
    }

    argb32_t& operator+=(const argb32_t multiplier) {
        *this = *this + multiplier;
        return *this;
    }

    argb32_t operator+(const argb32_t offset) const {
        return {u8_add_sat(r, offset.r),
                u8_add_sat(g, offset.g),
                u8_add_sat(b, offset.b),
                u8_add_sat(a, offset.a)};
    }

    constexpr argb32_t operator|(uint32_t mask) const {
        return argb32_t(argb | mask);
    }

    constexpr argb32_t operator&(uint32_t mask) const {
        return argb32_t(argb & mask);
    }

    [[nodiscard]]
    constexpr float af() const { return float(a) / 255.0f; }

    constexpr void af(float value) { a = uint8_t(static_cast<uint16_t>(value * 255.0f) & 0xFFu); }

//    inline argb32_t& operator=(const argb32_t& rhs) {
//        argb = rhs.argb;
//        return *this;
//    }

//    template<typename T>
//    explicit operator Vec<4, T>() {
//        return Vec<4, T>{T(r) / 255, T(g) / 255, T(b) / 255, T(a) / 255};
//    }

//    template<typename T>
//    explicit operator Vec<3, T>() {
//        return Vec<3, T>{T(r) / 255, T(g) / 255, T(b) / 255};
//    }

    constexpr bool operator==(const argb32_t& v) const {
        return argb == v.argb;
    }

    constexpr bool operator!=(const argb32_t& v) const {
        return argb != v.argb;
    }
};

inline constexpr argb32_t argb32_t::zero{0x0u};
inline constexpr argb32_t argb32_t::one{0xFFFFFFFFu};
inline constexpr argb32_t argb32_t::black{0xFF000000u};

//inline constexpr abgr32_t abgr32_t::zero{0x0u};
//inline constexpr abgr32_t abgr32_t::one{0xFFFFFFFFu};
//inline constexpr abgr32_t abgr32_t::black{0xFF000000u};

constexpr argb32_t operator "" _argb(unsigned long long n) noexcept {
    return argb32_t(0xFFFFFFFFu & n);
}

constexpr argb32_t operator "" _rgb(unsigned long long n) noexcept {
    return argb32_t(0xFF000000u | (0x00FFFFFFu & n));
}

inline argb32_t lerp(argb32_t begin, argb32_t end, float t) {
    auto r = static_cast<uint32_t>(t * 1024u);
    uint32_t ri = 1024u - r;
    return {uint8_t((begin.r * ri + end.r * r) >> 10u),
            uint8_t((begin.g * ri + end.g * r) >> 10u),
            uint8_t((begin.b * ri + end.b * r) >> 10u),
            uint8_t((begin.a * ri + end.a * r) >> 10u)};
}

}