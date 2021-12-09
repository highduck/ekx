#pragma once

#include <cstdint>
#include <array>
#include "Vec.hpp"

namespace ek {

template<unsigned N, typename T>
struct Rect final {
    using vec_type = Vec<N, T>;
    vec_type position;
    vec_type size;
    static_assert(sizeof(vec_type) == sizeof(T) * N, "struct has extra padding");
};

template<typename T>
struct Rect<2, T> final {

    using self_type = Rect<2, T>;
    using vec_type = Vec<2, T>;

    static constexpr unsigned dim = 2; // N
    static_assert(sizeof(vec_type) == sizeof(T) * dim, "struct has extra padding");

    static constexpr unsigned corners_count = 4; // 2 ^ N

#include <ek/math/internal/compiler_unsafe_begin.h>

    union {
        struct {
            vec_type position;
            vec_type size;
        };
        struct {
            T x;
            T y;
            T width;
            T height;
        };
    };

#include <ek/math/internal/compiler_unsafe_end.h>

    constexpr Rect() noexcept: x{0},
                               y{0},
                               width{0},
                               height{0} {

    }

    constexpr Rect(T x_, T y_ = 0, T width_ = 0, T height_ = 0) noexcept: x{x_},
                                                                          y{y_},
                                                                          width{width_},
                                                                          height{height_} {

    }

    template<typename S>
    constexpr Rect(S x_, S y_ = 0, S width_ = 0, S height_ = 0) noexcept: x{(T)x_},
                                                                          y{(T)y_},
                                                                          width{(T)width_},
                                                                          height{(T)height_} {

    }

    constexpr Rect(const vec_type& position_, const vec_type& size_) noexcept: position{position_},
                                                                               size{size_} {

    }

    template<typename S>
    constexpr explicit Rect(const Rect<2, S>& box_) noexcept :
            x{static_cast<T>(box_.x)},
            y{static_cast<T>(box_.y)},
            width{static_cast<T>(box_.width)},
            height{static_cast<T>(box_.height)} {

    }

    inline void set(T x_ = 0, T y_ = 0, T width_ = 0, T height_ = 0) {
        x = x_;
        y = y_;
        width = width_;
        height = height_;
    }

    inline void set(const self_type& other) {
        x = other.x;
        y = other.y;
        width = other.width;
        height = other.height;
    }

    inline void getCorners(vec_type out[4]) const {
        const T r = x + width;
        const T b = y + height;
        out[0] = {x, y};
        out[1] = {r, y};
        out[2] = {r, b};
        out[3] = {x, b};
    }

    inline T* data() {
        return reinterpret_cast<T*>(this);
    }

    [[nodiscard]]
    inline const T* data() const {
        return reinterpret_cast<const T*>(this);
    }

    [[nodiscard]]
    constexpr T left() const { return x; }

    constexpr void left(T v) { x = v; };

    [[nodiscard]]
    constexpr T top() const { return y; }

    constexpr void top(T v) { y = v; }

    [[nodiscard]]
    constexpr T right() const { return x + width; }

    constexpr void right(T v) { x = v - width; }

    [[nodiscard]]
    constexpr T bottom() const { return y + height; }

    constexpr void bottom(T v) { y = v - height; }

    [[nodiscard]]
    constexpr vec_type right_bottom() const noexcept {
        return position + size;
    }

    inline void center(const vec_type& center) {
        center_x(center.x);
        center_y(center.y);
    }

    [[nodiscard]]
    constexpr vec_type center() const { return {center_x(), center_y()}; }

    [[nodiscard]]
    constexpr T center_x() const { return x + width / 2; }

    constexpr void center_x(T v) { x = v - width / 2; }

    [[nodiscard]]
    constexpr T center_y() const { return y + height / 2; }

    constexpr void center_y(T v) { y = v - height / 2; }

    [[nodiscard]]
    constexpr bool empty() const { return width < Math::epsilon<T>() || height < Math::epsilon<T>(); }

    [[nodiscard]]
    constexpr vec_type relative(const vec_type& rel_coord) const {
        return vec_type{
                static_cast<T>(x + rel_coord.x * width),
                static_cast<T>(y + rel_coord.y * height)
        };
    }

    constexpr bool operator==(const self_type& a) const {
        return (x == a.x && y == a.y && width == a.width && height == a.height);
    }

    constexpr bool operator!=(const self_type& a) const {
        return (x != a.x || y != a.y || width != a.width || height != a.height);
    }

    // TODO: simplify
    [[nodiscard]]
    bool contains(T px, T py) const;

    [[nodiscard]]
    bool contains(vec_type point) const;

    [[nodiscard]]
    bool overlaps(const self_type& other) const;

    [[nodiscard]]
    self_type scale(float sx, float sy) const;

    [[nodiscard]]
    self_type scale(Vec2f sc) const {
        return scale(sc.x, sc.y);
    }

    template<typename S>
    constexpr Rect<2, S> operator/(const Vec<2, S>& vec) const {
        return Rect<2, S>{
                x / vec.x,
                y / vec.y,
                width / vec.x,
                height / vec.y
        };
    }

    template<typename S>
    constexpr Rect<2, S> operator/(S scalar) const {
        return Rect<2, S>{
                x / scalar,
                y / scalar,
                width / scalar,
                height / scalar
        };
    }

    template<typename S>
    constexpr Rect<2, S> operator*(S scalar) const {
        return Rect<2, S>{
                x * scalar,
                y * scalar,
                width * scalar,
                height * scalar
        };
    }

    constexpr Rect<2, T> operator*(const Vec<2, T>& scale) const {
        return Rect<2, T>{
                x * scale.x,
                y * scale.y,
                width * scale.x,
                height * scale.y
        };
    }

    constexpr self_type& operator*=(T scalar) {
        *this = *this * scalar;
        return *this;
    }

    static const self_type zero_one;
    static const self_type zero;
};

template<typename T>
constexpr Rect<2, T> Rect<2, T>::zero_one{0, 0, 1, 1};

template<typename T>
constexpr Rect<2, T> Rect<2, T>::zero{0, 0, 0, 0};

template<typename T>
using Rect2 = Rect<2, T>;

using Rect2f = Rect2<float>;
using Rect2i = Rect2<int>;
using Rect2u = Rect2<unsigned>;

template<typename T, unsigned N>
inline Rect<N, T> translate(const Rect<N, T>& box, const Vec <N, T>& offset) {
    return Rect<N, T>{box.position + offset, box.size};
}

template<typename T>
bool Rect<2, T>::contains(T px, T py) const {
    return x <= px && px <= (x + width) && y <= py && py <= (y + height);
}

template<typename T>
bool Rect<2, T>::contains(const Vec<2, T> point) const {
    return contains(point.x, point.y);
}

template<typename T>
bool Rect<2, T>::overlaps(const Rect<2, T>& other) const {
    return x <= other.right() && other.x <= right() && y <= other.bottom() && other.y <= bottom();
}

template<typename T>
Rect<2, T> Rect<2, T>::scale(float sx, float sy) const {
    Rect2f r{x * sx, y * sy, width * sx, height * sy};
    if (r.width < 0) {
        r.x += r.width;
        r.width = -r.width;
    }
    if (r.height < 0) {
        r.y += r.height;
        r.height = -r.height;
    }
    return r;
}

template<typename T, unsigned N>
Rect<N, T> lerp(const Rect<N, T>& begin, const Rect<N, T>& end, T t) {
    return {lerp(begin.position, end.position, t),
            lerp(begin.size, end.size, t)};
}

template<typename T, unsigned N>
Rect<N, T> expand(const Rect<N, T>& box, const Vec <N, T>& amount) {
    return Rect<N, T>{box.position - amount, box.size + T{2} * amount};
}

template<typename T, unsigned N>
Rect<N, T> expand(const Rect<N, T>& box, T scalar) {
    return expand(box, scalar * Vec<N, T>::one);
}

template<typename T, unsigned N>
constexpr Rect<N, T> min_max_box(const Vec <N, T>& min, const Vec <N, T>& max) noexcept {
    return {min, max - min};
}

template<typename T, unsigned N>
constexpr Rect<N, T> points_box(const Vec <N, T>& a, const Vec <N, T>& b) noexcept {
    return min_max_box(min_components(a, b), max_components(a, b));
}

template<typename T, unsigned N>
Rect<N, T> clamp_bounds(const Rect<N, T>& a, const Rect<N, T>& b) {
    return min_max_box(max_components(a.position, b.position),
                       min_components(a.right_bottom(), b.right_bottom()));
}

template<typename T, unsigned N>
Rect<N, T> combine(const Rect<N, T>& a, const Rect<N, T>& b) {
    return min_max_box(min_components(a.position, b.position),
                       max_components(a.right_bottom(), b.right_bottom()));
}

}