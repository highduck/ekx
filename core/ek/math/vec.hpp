#pragma once

#include "common.hpp"
#include <cstdint>
#include <cassert>
#include <algorithm>

namespace ek {

#define MAKE_ARRAY_ACCESS \
inline T* data() {\
return reinterpret_cast<T*>(this);\
}\
inline const T* data() const {\
    return reinterpret_cast<const T*>(this);\
}\
inline T& operator[](const unsigned index) {\
    assert(index >= 0 && index < dim);\
    return data()[index];\
}\
inline const T& operator[](const unsigned index) const {\
    assert(index >= 0 && index < dim);\
    return data()[index];\
}

template<unsigned N, typename T>
struct vec_t {

    constexpr static unsigned dim = N;
    static_assert(sizeof(vec_t<N, T>) == sizeof(T) * dim, "struct has extra padding");

    using self_type = vec_t<N, T>;

    T elements[N];

    MAKE_ARRAY_ACCESS
};

template<typename T>
using vec2_t = vec_t<2, T>;
using float2 = vec_t<2, float>;
using int2 = vec_t<2, int>;

template<typename T>
using vec3_t = vec_t<3, T>;
using float3 = vec_t<3, float>;
using int3 = vec_t<3, int>;

template<typename T>
using vec4_t = vec_t<4, T>;
using float4 = vec_t<4, float>;
using int4 = vec_t<4, int>;

template<typename T>
struct vec_t<2, T> {

    constexpr static unsigned dim = 2;
    using self_type = vec_t<2, T>;

    static const self_type zero;
    static const self_type one;

    T x;
    T y;

    constexpr vec_t() noexcept:
            x{0},
            y{0} {

    }

    template<typename S>
    inline explicit constexpr vec_t(vec2_t<S> other) noexcept:
            x{static_cast<T>(other.x)},
            y{static_cast<T>(other.y)} {

    }

    inline constexpr vec_t(T x_, T y_) noexcept:
            x{x_},
            y{y_} {

    }

    inline self_type operator-() const { return self_type{-x, -y}; }

    inline self_type operator-(const self_type& v) const { return self_type{x - v.x, y - v.y}; }

    inline self_type operator+(const self_type& v) const { return self_type{x + v.x, y + v.y}; }

    inline self_type operator*(const self_type& v) const { return self_type{x * v.x, y * v.y}; }

    inline self_type operator/(const self_type& v) const { return self_type{x / v.x, y / v.y}; }

    inline self_type operator*(T scalar) const { return self_type{x * scalar, y * scalar}; }

    inline self_type operator/(T scalar) const {
        T inv = T{1} / scalar;
        return self_type{x * inv, y * inv};
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

    MAKE_ARRAY_ACCESS
};

template<typename T>
inline const vec_t<2, T> vec_t<2, T>::zero{0, 0};

template<typename T>
inline const vec_t<2, T>vec_t<2, T>::one{1, 1};

template<typename T>
inline vec_t<2, T> perpendicular(vec_t<2, T> direction) {
    return {-direction.y, direction.x};
}

template<typename T>
struct vec_t<3, T> {

    constexpr static unsigned dim = 3;
    using self_type = vec_t<3, T>;

    static const self_type zero;
    static const self_type one;

    T x;
    T y;
    T z;

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

    explicit vec_t(const vec_t<2, T>& v2) noexcept
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

    inline vec_t<2, T> xy() const {
        return vec_t<2, T>{x, y};
    }

    MAKE_ARRAY_ACCESS
};

template<typename T>
inline const vec_t<3, T> vec_t<3, T>::zero{0, 0, 0};

template<typename T>
inline const vec_t<3, T> vec_t<3, T>::one{1, 1, 1};

template<typename T>
struct vec_t<4, T> {
    constexpr static unsigned dim = 4;
    using self_type = vec_t<4, T>;

    static const self_type zero;
    static const self_type one;

    T x;
    T y;
    T z;
    T w;

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

    vec_t(const vec_t<3, T>& v, T w_) noexcept
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

    MAKE_ARRAY_ACCESS
};

template<typename T>
inline const vec_t<4, T> vec_t<4, T>::zero{0, 0, 0, 0};
template<typename T>
inline const vec_t<4, T> vec_t<4, T>::one{1, 1, 1, 1};

template<typename T, unsigned N>
inline static vec_t<N, T> operator*(T scalar, const vec_t<N, T>& v) {
    return v * scalar;
}

template<typename T, unsigned N>
inline T dot(const vec_t<N, T>& a, const vec_t<N, T>& b) {
    T sum{0};
    for (unsigned i = 0; i < N; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

template<typename T, unsigned N>
bool equals(const vec_t<N, T>& a, const vec_t<N, T>& b) {
    for (unsigned i = 0; i < N; ++i) {
        if (!math::equals(a[i], b[i])) {
            return false;
        }
    }
    return true;
}

template<typename T, unsigned N>
inline T length_sqr(const vec_t<N, T>& a) {
    return dot(a, a);
}

template<typename T, unsigned N>
inline T length(const vec_t<N, T>& a) {
    return sqrt(length_sqr(a));
}

template<typename T>
inline T length_sqr(vec_t<2, T> a) {
    return a.x * a.x + a.y * a.y;
}

template<typename T>
inline T length(vec_t<2, T> a) {
    return sqrt(a.x * a.x + a.y * a.y);
}

template<typename T, unsigned N>
inline T distance(const vec_t<N, T>& a, const vec_t<N, T>& b) {
    return length(a - b);
}

template<typename T, unsigned N>
inline T distance_sqr(const vec_t<N, T>& a, const vec_t<N, T>& b) {
    return length_sqr(a - b);
}

template<typename T, unsigned N>
inline vec_t<N, T> reflect(const vec_t<N, T>& direction, const vec_t<N, T>& normal) {
    // factor = -2 * dot(normal, direction)
    // reflected = factor * normal + direction;
    return direction - T{1} * dot(normal, direction) * normal;
}

inline vec2_t<float> normalize_2f(vec2_t<float> a) {
    return a * math::Q_rsqrt(a.x * a.x + a.y * a.y);
}

inline vec3_t<float> normalize_3f(vec3_t<float> a) {
    return a * math::Q_rsqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

template<unsigned N>
inline vec_t<N, float> normalize(const vec_t<N, float>& a) {
    return a * math::Q_rsqrt(dot(a, a));
}

template<typename T, unsigned N>
inline vec_t<N, T> normalize(const vec_t<N, T>& a) {
    return a / sqrt(dot(a, a));
}

template<typename T, unsigned N>
inline vec_t<N, T> lerp(const vec_t<N, T>& begin, const vec_t<N, T>& end, T t) {
    return (T{1} - t) * begin + t * end;
}

// specialized functions
template<typename T>
inline vec_t<3, T> cross(const vec_t<3, T>& a, const vec_t<3, T>& b) {
    return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
    };
}

template<typename T>
inline T sign(const vec_t<2, T>& p_1, const vec_t<2, T>& p_2, const vec_t<2, T>& p_3) {
    return (p_1.x - p_3.x) * (p_2.y - p_3.y) - (p_2.x - p_3.x) * (p_1.y - p_3.y);
}

template<typename T, unsigned N>
constexpr vec_t<N, T> min_components(const vec_t<N, T>& a, const vec_t<N, T>& b) noexcept {
    vec_t<N, T> r;
    for (unsigned i = 0; i < N; ++i) {
        r[i] = std::min(a[i], b[i]);
    }
    return r;
}

template<typename T, unsigned N>
constexpr vec_t<N, T> max_components(const vec_t<N, T>& a, const vec_t<N, T>& b) noexcept {
    vec_t<N, T> r;
    for (unsigned i = 0; i < N; ++i) {
        r[i] = std::max(a[i], b[i]);
    }
    return r;
}

}
