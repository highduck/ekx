#pragma once

#include <cstdint>
#include "vec.hpp"

namespace ek {

namespace details {

inline const uint8_t* init_clamp_255() noexcept {
    static uint8_t arr[512];
    for (int i = 0; i < 256; ++i) arr[i] = (uint8_t) i;
    for (int i = 256; i < 512; ++i) arr[i] = 0xFF;
    return arr;
}

inline const uint8_t* clamp_255 = init_clamp_255();

}

struct abgr32_t final {

    static const abgr32_t zero;
    static const abgr32_t one;
    static const abgr32_t black;

#include <ek/math/internal/compiler_unsafe_begin.h>

    union {
        uint32_t abgr;
        struct {
            uint8_t r, g, b, a;
        };
    };

#include <ek/math/internal/compiler_unsafe_end.h>


    inline abgr32_t() noexcept {

    }

    inline constexpr abgr32_t(uint32_t abgr_) noexcept
            : abgr{abgr_} {

    }

    inline abgr32_t(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_) noexcept
            : r{r_}, g{g_}, b{b_}, a{a_} {}

    inline abgr32_t operator*(abgr32_t multiplier) const {
        return abgr32_t{
                static_cast<uint8_t>((r * multiplier.r * 258u) >> 16u),
                static_cast<uint8_t>((g * multiplier.g * 258u) >> 16u),
                static_cast<uint8_t>((b * multiplier.b * 258u) >> 16u),
                static_cast<uint8_t>((a * multiplier.a * 258u) >> 16u)
        };
    }

    abgr32_t& operator*=(const abgr32_t multiplier) {
        *this = *this * multiplier;
        return *this;
    }

    abgr32_t& operator+=(const abgr32_t multiplier) {
        *this = *this + multiplier;
        return *this;
    }

    inline abgr32_t operator+(const abgr32_t offset) const {
        using details::clamp_255;
        return abgr32_t(
                clamp_255[r + offset.r],
                clamp_255[g + offset.g],
                clamp_255[b + offset.b],
                clamp_255[a + offset.a]
        );
    }

    inline abgr32_t operator|(uint32_t mask) const {
        return abgr32_t{abgr | mask};
    }

    inline abgr32_t operator&(uint32_t mask) const {
        return abgr32_t{abgr & mask};
    }

    [[nodiscard]] inline float af() const { return float(a) / 255.0f; }

    inline void af(float value) { a = uint8_t(static_cast<uint16_t>(value * 255.0f) & 0xFFu); }

    [[nodiscard]] inline abgr32_t scaleAlpha(float scale) const {
        return abgr32_t{(static_cast<uint8_t>(scale * a) << 24u) | (abgr & 0xFFFFFFu)};
    }

    template<typename T>
    explicit operator vec_t<4, T>() {
        return vec_t<4, T>{T(r) / 255, T(g) / 255, T(b) / 255, T(a) / 255};
    }
};

struct argb32_t final {

    static const argb32_t zero;
    static const argb32_t one;
    static const argb32_t black;

#include <ek/math/internal/compiler_unsafe_begin.h>

    union {
        uint32_t argb;
        struct {
            uint8_t b, g, r, a;
        };
    };

#include <ek/math/internal/compiler_unsafe_end.h>

    inline argb32_t() noexcept = default;
//    constexpr argb32_t(const argb32_t&) noexcept = default;
//    constexpr argb32_t(argb32_t&&) noexcept = default;

    inline argb32_t(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_) noexcept
            : b{b_}, g{g_}, r{r_}, a{a_} {}

//        template<typename T>
    inline argb32_t(float r_, float g_, float b_, float a_ = 1.0f) noexcept
            : b{static_cast<uint8_t>(int(b_ * 255) & 0xFFu)},
              g{static_cast<uint8_t>(int(g_ * 255) & 0xFFu)},
              r{static_cast<uint8_t>(int(r_ * 255) & 0xFFu)},
              a{static_cast<uint8_t>(int(a_ * 255) & 0xFFu)} {
    }

    template<typename T>
    explicit inline argb32_t(const vec_t<4, T>& vector)
            :argb32_t(vector.x, vector.y, vector.z, vector.w) {
    }

    template<typename T>
    explicit inline argb32_t(const vec_t<3, T>& vector, float alpha = 1.0f)
            :argb32_t(vector.x, vector.y, vector.z, alpha) {
    }

    inline constexpr explicit argb32_t(uint32_t argb_) noexcept
            : argb{argb_} {}

    inline argb32_t(uint32_t rgb_, float alpha)
            : argb{rgb_ & 0xFFFFFFu} {
        af(alpha);
    }

    [[nodiscard]] inline constexpr abgr32_t bgr() const noexcept {
        return abgr32_t((static_cast<uint32_t>(b) << 16u) | (argb & 0x0000FF00u) | r);
    }

    [[nodiscard]] inline constexpr abgr32_t abgr() const noexcept {
        return abgr32_t{(argb & 0xFF00FF00u) | (static_cast<uint32_t>(b) << 16u) | r};
    }

    inline constexpr operator abgr32_t() const noexcept {
        return abgr();
    }

    inline argb32_t operator*(argb32_t multiplier) const {
        return argb32_t(
                static_cast<uint8_t>((r * multiplier.r * 258u) >> 16u),
                static_cast<uint8_t>((g * multiplier.g * 258u) >> 16u),
                static_cast<uint8_t>((b * multiplier.b * 258u) >> 16u),
                static_cast<uint8_t>((a * multiplier.a * 258u) >> 16u)
        );
    }

    argb32_t& operator*=(const argb32_t multiplier) {
        *this = *this * multiplier;
        return *this;
    }

    argb32_t& operator+=(const argb32_t multiplier) {
        *this = *this + multiplier;
        return *this;
    }

    inline argb32_t operator+(const argb32_t offset) const {
        using details::clamp_255;
        return argb32_t(
                clamp_255[r + offset.r],
                clamp_255[g + offset.g],
                clamp_255[b + offset.b],
                clamp_255[a + offset.a]
        );
    }

    inline argb32_t operator|(uint32_t mask) const {
        return argb32_t(argb | mask);
    }

    inline argb32_t operator&(uint32_t mask) const {
        return argb32_t(argb & mask);
    }

    inline float af() const { return float(a) / 255.0f; }

    inline void af(float value) { a = uint8_t(static_cast<uint16_t>(value * 255.0f) & 0xFFu); }

//    inline argb32_t& operator=(const argb32_t& rhs) {
//        argb = rhs.argb;
//        return *this;
//    }

    template<typename T>
    explicit operator vec_t<4, T>() {
        return vec_t<4, T>{T(r) / 255, T(g) / 255, T(b) / 255, T(a) / 255};
    }

    template<typename T>
    explicit operator vec_t<3, T>() {
        return vec_t<3, T>{T(r) / 255, T(g) / 255, T(b) / 255};
    }

    inline bool operator==(const argb32_t& v) const {
        return argb == v.argb;
    }

    inline bool operator!=(const argb32_t& v) const {
        return argb != v.argb;
    }
};

constexpr argb32_t argb32_t::zero{0x0u};
constexpr argb32_t argb32_t::one{0xFFFFFFFFu};
constexpr argb32_t argb32_t::black{0xFF000000u};

constexpr abgr32_t abgr32_t::zero{0x0u};
constexpr abgr32_t abgr32_t::one{0xFFFFFFFFu};
constexpr abgr32_t abgr32_t::black{0xFF000000u};

constexpr argb32_t operator "" _argb(unsigned long long n) noexcept {
    return argb32_t(0xFFFFFFFFu & n);
}

constexpr argb32_t operator "" _rgb(unsigned long long n) noexcept {
    return argb32_t(0xFF000000u | (0x00FFFFFFu & n));
}

inline argb32_t lerp(argb32_t begin, argb32_t end, float t) {
    auto r = static_cast<uint32_t>(t * 1024u);
    uint32_t ri = 1024u - r;
    return argb32_t(uint8_t((begin.r * ri + end.r * r) >> 10u),
                    uint8_t((begin.g * ri + end.g * r) >> 10u),
                    uint8_t((begin.b * ri + end.b * r) >> 10u),
                    uint8_t((begin.a * ri + end.a * r) >> 10u));
}

inline abgr32_t lerp(abgr32_t begin, abgr32_t end, float t) {
    auto r = static_cast<uint32_t>(t * 1024u);
    uint32_t ri = 1024u - r;
    return abgr32_t((begin.r * ri + end.r * r) >> 10u,
                    (begin.g * ri + end.g * r) >> 10u,
                    (begin.b * ri + end.b * r) >> 10u,
                    (begin.a * ri + end.a * r) >> 10u);
}


struct ColorMod32 {
    abgr32_t scale = abgr32_t::one;
    abgr32_t offset = abgr32_t::zero;

    ColorMod32() = default;

    explicit ColorMod32(argb32_t scale_, argb32_t offset_ = argb32_t::zero) :
            scale{scale_},
            offset{offset_} {
    }

    explicit ColorMod32(abgr32_t scale_, abgr32_t offset_ = abgr32_t::zero) :
            scale{scale_},
            offset{offset_} {
    }

    ColorMod32 operator*(ColorMod32 r) const {
        using details::clamp_255;
        return ColorMod32{
                r.scale.abgr != 0xFFFFFFFF ? scale * r.scale : scale,
                r.offset.abgr != 0 ? abgr32_t{clamp_255[offset.r + ((r.offset.r * scale.r * 258u) >> 16u)],
                                              clamp_255[offset.g + ((r.offset.g * scale.g * 258u) >> 16u)],
                                              clamp_255[offset.b + ((r.offset.b * scale.b * 258u) >> 16u)],
                                              clamp_255[offset.a + r.offset.a]} : offset
        };
    }

    inline static void multiply(ColorMod32 l, ColorMod32 r, ColorMod32& out) {
        using details::clamp_255;
//        out.scale = r.scale.abgr != 0xFFFFFFFF ? l.scale * r.scale : l.scale;
//        out.offset = r.offset.abgr != 0 ? abgr32_t{clamp_255[l.offset.r + ((r.offset.r * l.scale.r * 258u) >> 16u)],
//                                                   clamp_255[l.offset.g + ((r.offset.g * l.scale.g * 258u) >> 16u)],
//                                                   clamp_255[l.offset.b + ((r.offset.b * l.scale.b * 258u) >> 16u)],
//                                                   clamp_255[l.offset.a + r.offset.a]} : l.offset;

        out.scale = l.scale * r.scale;
        out.offset = abgr32_t{clamp_255[l.offset.r + ((r.offset.r * l.scale.r * 258u) >> 16u)],
                              clamp_255[l.offset.g + ((r.offset.g * l.scale.g * 258u) >> 16u)],
                              clamp_255[l.offset.b + ((r.offset.b * l.scale.b * 258u) >> 16u)],
                              clamp_255[l.offset.a + r.offset.a]};
    }

    inline void setAlpha(float alpha) {
        scale.af(alpha);
    }

    [[nodiscard]] inline float getAlpha() const {
        return scale.af();
    }

    inline void setAdditive(float alpha) {
        offset.af(alpha);
    }

    [[nodiscard]] inline float getAdditive() const {
        return offset.af();
    }

};

}