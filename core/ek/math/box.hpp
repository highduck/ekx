#pragma once

#include <cstdint>
#include <array>
#include "vec.hpp"

namespace ek {

template<unsigned N, typename T>
struct box_t final {
    using vec_type = vec_t<N, T>;
    vec_type position;
    vec_type size;
    static_assert(sizeof(vec_type) == sizeof(T) * N, "struct has extra padding");
};

template<typename T>
struct box_t<2, T> {

    using self_type = box_t<2, T>;
    using vec_type = vec_t<2, T>;

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

    box_t() noexcept
            : x{0},
              y{0},
              width{0},
              height{0} {

    }

    box_t(T x_, T y_ = 0, T width_ = 0, T height_ = 0) noexcept
            : x{x_},
              y{y_},
              width{width_},
              height{height_} {

    }

    box_t(const vec_type& position_, const vec_type& size_) noexcept
            : position{position_},
              size{size_} {

    }

    template<typename S>
    explicit box_t(const box_t<2, S>& box_) noexcept :
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

    inline std::array<vec_type, corners_count> corners() const {
        return {
                vec_type{x, y},
                vec_type{x + width, y},
                vec_type{x + width, y + height},
                vec_type{x, y + height}
        };
    }

    inline T* data() {
        return reinterpret_cast<T*>(this);
    }

    [[nodiscard]]
    inline const T* data() const {
        return reinterpret_cast<const T*>(this);
    }

    inline T left() const { return x; }

    inline void left(T v) { x = v; };

    inline T top() const { return y; }

    inline void top(T v) { y = v; }

    inline T right() const { return x + width; }

    inline void right(T v) { x = v - width; }

    inline float bottom() const { return y + height; }

    inline void bottom(T v) { y = v - height; }

    constexpr vec_type right_bottom() const noexcept {
        return position + size;
    }

    inline void center(const vec_type& center) {
        center_x(center.x);
        center_y(center.y);
    }

    inline vec_type center() const { return {center_x(), center_y()}; }

    inline T center_x() const { return x + width / 2; }

    inline void center_x(T v) { x = v - width / 2; }

    inline T center_y() const { return y + height / 2; }

    inline void center_y(T v) { y = v - height / 2; }

    inline bool empty() const { return width < math::epsilon<T>() || height < math::epsilon<T>(); }

    inline vec_type relative(const vec_type& rel_coord) const {
        return vec_type{
                static_cast<T>(x + rel_coord.x * width),
                static_cast<T>(y + rel_coord.y * height)
        };
    }

    bool operator==(const self_type& a) const {
        return (x == a.x && y == a.y && width == a.width && height == a.height);
    }

    inline bool operator!=(const self_type& a) const {
        return (x != a.x || y != a.y || width != a.width || height != a.height);
    }

    // TODO: simplify
    bool contains(float px, float py) const;

    bool contains(const vec_type& point) const;

    bool overlaps(const self_type& other) const;

    self_type scale(float sx, float sy) const;

    inline self_type scale(float2 sc) const {
        return scale(sc.x, sc.y);
    }

    template<typename S>
    inline box_t<2, S> operator/(const vec_t<2, S>& vec) const {
        return box_t<2, S>{
                x / vec.x,
                y / vec.y,
                width / vec.x,
                height / vec.y
        };
    }

    template<typename S>
    inline box_t<2, S> operator/(S scalar) const {
        return box_t<2, S>{
                x / scalar,
                y / scalar,
                width / scalar,
                height / scalar
        };
    }

    template<typename S>
    inline box_t<2, S> operator*(S scalar) const {
        return box_t<2, S>{
                x * scalar,
                y * scalar,
                width * scalar,
                height * scalar
        };
    }

    inline box_t<2, T> operator*(const vec_t<2, T>& scale) const {
        return box_t<2, T>{
                x * scale.x,
                y * scale.y,
                width * scale.x,
                height * scale.y
        };
    }

    inline self_type& operator*=(T scalar) {
        *this = *this * scalar;
        return *this;
    }

    static const self_type zero_one;
    static const self_type zero;
};

template<typename T>
inline const box_t<2, T> box_t<2, T>::zero_one{0, 0, 1, 1};

template<typename T>
inline const box_t<2, T> box_t<2, T>::zero{0, 0, 0, 0};

template<typename T>
using rect_t = box_t<2, T>;

using rect_f = rect_t<float>;
using rect_i = rect_t<int32_t>;
using rect_u = rect_t<uint32_t>;

template<typename T, unsigned N>
inline box_t<N, T> translate(const box_t<N, T>& box, const vec_t <N, T>& offset) {
    return box_t<N, T>{box.position + offset, box.size};
}

template<typename T>
bool box_t<2, T>::contains(float px, float py) const {
    // A little more complicated than usual due to proper handling of negative widths/heights
    px -= x;
    if (width >= 0) {
        if (px < 0 || px > width) {
            return false;
        }
    } else if (px > 0 || px < width) {
        return false;
    }

    py -= y;
    if (height >= 0) {
        if (py < 0 || py > height) {
            return false;
        }
    } else if (py > 0 || py < height) {
        return false;
    }

    return true;
}

template<typename T>
bool box_t<2, T>::contains(const vec_t<2, T>& point) const {
    return contains(point.x, point.y);
}

template<typename T>
bool box_t<2, T>::overlaps(const box_t<2, T>& other) const {
    return x <= other.right() && other.x <= right() && y <= other.bottom() && other.y <= bottom();
}

template<typename T>
box_t<2, T> box_t<2, T>::scale(float sx, float sy) const {
    rect_f r{x * sx, y * sy, width * sx, height * sy};
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
box_t<N, T> lerp(const box_t<N, T>& begin, const box_t<N, T>& end, T t) {
    return {lerp(begin.position, end.position, t),
            lerp(begin.size, end.size, t)};
}

template<typename T, unsigned N>
box_t<N, T> expand(const box_t<N, T>& box, const vec_t <N, T>& amount) {
    return box_t<N, T>{box.position - amount, box.size + T{2} * amount};
}

template<typename T, unsigned N>
box_t<N, T> expand(const box_t<N, T>& box, T scalar) {
    return expand(box, scalar * vec_t<N, T>::one);
}

template<typename T, unsigned N>
constexpr box_t<N, T> min_max_box(const vec_t <N, T>& min, const vec_t <N, T>& max) noexcept {
    return {min, max - min};
}

template<typename T, unsigned N>
constexpr box_t<N, T> points_box(const vec_t <N, T>& a, const vec_t <N, T>& b) noexcept {
    return min_max_box(min_components(a, b), max_components(a, b));
}

template<typename T, unsigned N>
box_t<N, T> clamp_bounds(const box_t<N, T>& a, const box_t<N, T>& b) {
    return min_max_box(max_components(a.position, b.position),
                       min_components(a.right_bottom(), b.right_bottom()));
}

template<typename T, unsigned N>
box_t<N, T> combine(const box_t<N, T>& a, const box_t<N, T>& b) {
    return min_max_box(min_components(a.position, b.position),
                       max_components(a.right_bottom(), b.right_bottom()));
}

}