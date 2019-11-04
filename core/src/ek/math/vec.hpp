#pragma once

#include "common.hpp"
#include "vec_fwd.hpp"
#include <cstdint>
#include <algorithm>

namespace ek {

template<typename T, unsigned N>
struct vec_t {

    constexpr static unsigned dim = N;
    static_assert(sizeof(vec_t<T, dim>) == sizeof(T) * dim, "struct has extra padding");

    using self_type = vec_t<T, N>;
    T data_[N];
};

template<typename T>
struct vec_t<T, 2> {

    constexpr static unsigned dim = 2;
    using self_type = vec_t<T, 2>;

    inline static const self_type zero{0, 0};
    inline static const self_type one{1, 1};

#include <ek/math/internal/compiler_unsafe_begin.h>

    union {
        struct {
            T x, y;
        };
        T data_[dim];
    };

#include <ek/math/internal/compiler_unsafe_end.h>

    constexpr vec_t() noexcept
            : x{0},
              y{0} {

    }

    constexpr vec_t(T x_, T y_) noexcept
            : x{x_},
              y{y_} {

    }

    inline self_type operator-() const { return self_type(-x, -y); }

    inline self_type operator-(const self_type& v) const { return self_type(x - v.x, y - v.y); }

    inline self_type operator+(const self_type& v) const { return self_type(x + v.x, y + v.y); }

    inline self_type operator*(const self_type& v) const { return self_type(x * v.x, y * v.y); }

    inline self_type operator/(const self_type& v) const { return self_type(x / v.x, y / v.y); }

    inline self_type operator*(T scalar) const { return self_type(x * scalar, y * scalar); }

    inline self_type operator/(T scalar) const {
        T inv = T{1} / scalar;
        return self_type(x * inv, y * inv);
    }

    inline self_type& operator*=(T scalar) {
        *this = *this * scalar;
        return *this;
    }

    inline self_type& operator+=(const self_type& v) {
        *this = *this + v;
        return *this;
    }

    inline self_type& operator-=(const self_type& v) {
        *this = *this - v;
        return *this;
    }

    inline bool operator==(const self_type& a) const {
        return x == a.x && y == a.y;
    }

    inline bool operator!=(const self_type& a) const {
        return x != a.x || y != a.y;
    }
};

template<typename T>
inline vec_t<T, 2> perpendicular(const vec_t<T, 2>& direction) {
    return {-direction.y, direction.x};
}

template<typename T>
struct vec_t<T, 3> {

    constexpr static unsigned dim = 3;
    using self_type = vec_t<T, 3>;

    inline static const self_type zero{0, 0, 0};
    inline static const self_type one{1, 1, 1};

#include <ek/math/internal/compiler_unsafe_begin.h>

    union {
        struct {
            T x, y, z;
        };
        struct {
            T r, g, b;
        };
        T data_[dim];
    };

#include <ek/math/internal/compiler_unsafe_end.h>

    vec_t() noexcept
            : x{0},
              y{0},
              z{0} {

    }

    vec_t(T x_, T y_, T z_) noexcept
            : x{x_},
              y{y_},
              z{z_} {

    }

    explicit vec_t(const vec_t<T, 2>& v2) noexcept
            : x{v2.x},
              y{v2.y},
              z{0} {

    }

    inline self_type operator-(const self_type& v) const { return self_type(x - v.x, y - v.y, z - v.z); }

    inline self_type operator+(const self_type& v) const { return self_type(x + v.x, y + v.y, z + v.z); }

    inline self_type operator*(const self_type& v) const { return self_type(x * v.x, y * v.y, z * v.z); }

    inline self_type operator/(const self_type& v) const { return self_type(x / v.x, y / v.y, z / v.z); }

    inline self_type operator*(T scalar) const { return self_type(x * scalar, y * scalar, z * scalar); }

    inline self_type operator/(T scalar) const {
        T inv = T{1} / scalar;
        return self_type(x * inv, y * inv, z * inv);
    }

    inline self_type operator-() const { return self_type(-x, -y, -z); }

    inline self_type& operator*=(T scalar) {
        *this = *this * scalar;
        return *this;
    }

    inline self_type& operator/=(T scalar) {
        *this = *this / scalar;
        return *this;
    }

    inline self_type& operator+=(const self_type& v) {
        *this = *this + v;
        return *this;
    }

    inline self_type& operator-=(const self_type& v) {
        *this = *this - v;
        return *this;
    }

    inline self_type& operator*=(const self_type& v) {
        *this = *this * v;
        return *this;
    }

    inline bool operator==(const self_type& a) const {
        return x == a.x && y == a.y;
    }

    inline bool operator!=(const self_type& a) const {
        return x != a.x || y != a.y;
    }

    inline vec_t<T, 2> xy() const {
        return vec_t<T, 2>{x, y};
    }

    inline T& operator[](const unsigned index) {
        return data_[index];
    }

    inline const T& operator[](const unsigned index) const {
        return data_[index];
    }
};

template<typename T>
struct vec_t<T, 4> {
    constexpr static unsigned dim = 4;
    using self_type = vec_t<T, 4>;

    inline static const self_type zero{0, 0, 0, 0};
    inline static const self_type one{1, 1, 1, 1};

#include <ek/math/internal/compiler_unsafe_begin.h>

    union {
        struct {
            T x, y, z, w;
        };
        struct {
            T r, g, b, a;
        };
        T data_[dim];
    };

#include <ek/math/internal/compiler_unsafe_end.h>

    vec_t() noexcept
            : x{0},
              y{0},
              z{0},
              w{0} {

    }

    vec_t(T x_, T y_, T z_, T w_) noexcept
            : x{x_},
              y{y_},
              z{z_},
              w{w_} {

    }

    vec_t(const vec_t<T, 3>& v, T w_) noexcept
            : x{v.x},
              y{v.y},
              z{v.z},
              w{w_} {

    }

    inline self_type operator-() const { return self_type(-x, -y, -z, -w); }

    inline self_type operator-(const self_type& v) const { return self_type(x - v.x, y - v.y, z - v.z, w - v.w); }

    inline self_type operator+(const self_type& v) const { return self_type(x + v.x, y + v.y, z + v.z, w + v.w); }

    inline self_type operator*(const self_type& v) const { return self_type(x * v.x, y * v.y, z * v.z, w * v.w); }

    inline self_type operator/(const self_type& v) const { return self_type(x / v.x, y / v.y, z / v.z, w / v.w); }

    inline self_type operator*(T scalar) const {
        return self_type(x * scalar, y * scalar, z * scalar, w * scalar);
    }

    inline self_type operator/(T scalar) const {
        T inv = T{1} / scalar;
        return self_type(x * inv, y * inv, z * inv, w * inv);
    }

    inline self_type& operator*=(T scalar) {
        *this = *this * scalar;
        return *this;
    }

    inline self_type& operator/=(T scalar) {
        *this = *this / scalar;
        return *this;
    }

    inline self_type& operator*=(const self_type& v) {
        *this = *this * v;
        return *this;
    }

    inline self_type& operator/=(const self_type& v) {
        *this = *this / v;
        return *this;
    }

    inline self_type& operator+=(const self_type& v) {
        *this = *this + v;
        return *this;
    }

    inline self_type& operator-=(const self_type& v) {
        *this = *this - v;
        return *this;
    }

    inline bool operator==(const self_type& v) const {
        return x == v.x &&
               y == v.y &&
               z == v.z &&
               w == v.w;
    }

    inline bool operator!=(const self_type& v) const {
        return x != v.x ||
               y != v.y ||
               z != v.z ||
               w != v.w;
    }
};

template<typename T, unsigned N>
inline static vec_t<T, N> operator*(T scalar, const vec_t<T, N>& v) {
    return v * scalar;
}

template<typename T, unsigned N>
inline T dot(const vec_t<T, N>& a, const vec_t<T, N>& b) {
    T sum{0};
    for (unsigned i = 0; i < N; ++i) {
        sum += a.data_[i] * b.data_[i];
    }
    return sum;
}

template<typename T, unsigned N>
bool equals(const vec_t<T, N>& a, const vec_t<T, N>& b) {
    for (unsigned i = 0; i < N; ++i) {
        if (!math::equals(a.data_[i], b.data_[i])) {
            return false;
        }
    }
    return true;
}

template<typename T, unsigned N>
inline T length_sqr(const vec_t<T, N>& a) {
    return dot(a, a);
}

template<typename T, unsigned N>
inline T length(const vec_t<T, N>& a) {
    return sqrt(length_sqr(a));
}

template<typename T, unsigned N>
inline T distance(const vec_t<T, N>& a, const vec_t<T, N>& b) {
    return length(a - b);
}

template<typename T, unsigned N>
inline T distance_sqr(const vec_t<T, N>& a, const vec_t<T, N>& b) {
    return length_sqr(a - b);
}

template<typename T, unsigned N>
inline vec_t<T, N> reflect(const vec_t<T, N>& direction, const vec_t<T, N>& normal) {
    // factor = -2 * dot(normal, direction)
    // reflected = factor * normal + direction;
    return direction - T{1} * dot(normal, direction) * normal;
}

template<typename T, unsigned N>
inline vec_t<T, N> normalize(const vec_t<T, N>& a) {
    T len = length(a);
    if (len < math::epsilon<T>()) {
        return {};
    }
    return a * (T{1} / len);
}

template<typename T, unsigned N>
inline vec_t<T, N> lerp(const vec_t<T, N>& begin, const vec_t<T, N>& end, T t) {
    return (T{1} - t) * begin + t * end;
}

// specialized functions
template<typename T>
inline vec_t<T, 3> cross(const vec_t<T, 3>& a, const vec_t<T, 3>& b) {
    return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
    };
}

template<typename T>
inline T sign(const vec_t<T, 2>& p_1, const vec_t<T, 2>& p_2, const vec_t<T, 2>& p_3) {
    return (p_1.x - p_3.x) * (p_2.y - p_3.y) - (p_2.x - p_3.x) * (p_1.y - p_3.y);
}

template<typename T, unsigned N>
constexpr vec_t<T, N> min_components(const vec_t<T, N>& a, const vec_t<T, N>& b) noexcept {
    vec_t<T, N> r;
    for (unsigned i = 0; i < N; ++i) {
        r.data_[i] = std::min(a.data_[i], b.data_[i]);
    }
    return r;
}

template<typename T, unsigned N>
constexpr vec_t<T, N> max_components(const vec_t<T, N>& a, const vec_t<T, N>& b) noexcept {
    vec_t<T, N> r;
    for (unsigned i = 0; i < N; ++i) {
        r.data_[i] = std::max(a.data_[i], b.data_[i]);
    }
    return r;
}

}
