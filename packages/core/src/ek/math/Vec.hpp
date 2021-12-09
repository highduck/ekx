#pragma once

#include "Math.hpp"
#include <cstdint>
#include <ek/assert.h>
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
    EK_ASSERT(index >= 0 && index < dim);\
    return data()[index];\
}\
inline const T& operator[](const unsigned index) const {\
    EK_ASSERT(index >= 0 && index < dim);\
    return data()[index];\
}

template<unsigned N, typename T>
struct Vec {

    constexpr static unsigned dim = N;
    static_assert(sizeof(Vec<N, T>) == sizeof(T) * dim, "struct has extra padding");

    using self_type = Vec<N, T>;

    T elements[N];

    MAKE_ARRAY_ACCESS
};

template<typename T>
using Vec2 = Vec<2, T>;
using Vec2f = Vec<2, float>;
using Vec2i = Vec<2, int>;

template<typename T>
using Vec3 = Vec<3, T>;
using Vec3f = Vec<3, float>;
using Vec3i = Vec<3, int>;

template<typename T>
using Vec4 = Vec<4, T>;
using Vec4f = Vec<4, float>;
using Vec4i = Vec<4, int>;

template<typename T>
struct Vec<2, T> {

    constexpr static unsigned dim = 2;
    using self_type = Vec<2, T>;

    static const self_type zero;
    static const self_type one;

    T x;
    T y;

    constexpr Vec() noexcept: x{0}, y{0} {
    }

    constexpr explicit Vec(T xy) noexcept: x{xy}, y{xy} {
    }

    template<typename S>
    explicit constexpr Vec(Vec2<S> other) noexcept:
            x{static_cast<T>(other.x)},
            y{static_cast<T>(other.y)} {

    }

    constexpr Vec(T x_, T y_) noexcept: x{x_}, y{y_} {
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
inline const Vec<2, T> Vec<2, T>::zero{0, 0};

template<typename T>
inline const Vec<2, T>Vec<2, T>::one{1, 1};

template<typename T>
inline Vec<2, T> perpendicular(Vec<2, T> direction) {
    return {-direction.y, direction.x};
}

template<typename T>
struct Vec<3, T> {

    constexpr static unsigned dim = 3;
    using self_type = Vec<3, T>;

    static const self_type zero;
    static const self_type one;

    T x;
    T y;
    T z;

    constexpr Vec() noexcept: x{0},
                              y{0},
                              z{0} {

    }

    constexpr Vec(T x_, T y_, T z_) noexcept: x{x_},
                                              y{y_},
                                              z{z_} {

    }

    constexpr explicit Vec(const Vec<2, T>& v2) noexcept: x{v2.x},
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

    [[nodiscard]]
    constexpr Vec<2, T> xy() const {
        return Vec<2, T>{x, y};
    }

    MAKE_ARRAY_ACCESS
};

template<typename T>
inline const Vec<3, T> Vec<3, T>::zero{0, 0, 0};

template<typename T>
inline const Vec<3, T> Vec<3, T>::one{1, 1, 1};

template<typename T>
struct Vec<4, T> {
    constexpr static unsigned dim = 4;
    using self_type = Vec<4, T>;

    static const self_type zero;
    static const self_type one;

    T x;
    T y;
    T z;
    T w;

    constexpr Vec() noexcept: x{0},
                              y{0},
                              z{0},
                              w{0} {

    }

    constexpr Vec(T x_, T y_, T z_, T w_) noexcept: x{x_},
                                                    y{y_},
                                                    z{z_},
                                                    w{w_} {

    }

    constexpr Vec(const Vec<3, T>& v, T w_) noexcept: x{v.x},
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
inline const Vec<4, T> Vec<4, T>::zero{0, 0, 0, 0};
template<typename T>
inline const Vec<4, T> Vec<4, T>::one{1, 1, 1, 1};

template<typename T, unsigned N>
inline static Vec<N, T> operator*(T scalar, const Vec<N, T>& v) {
    return v * scalar;
}

template<typename T, unsigned N>
inline T dot(const Vec<N, T>& a, const Vec<N, T>& b) {
    T sum{0};
    for (unsigned i = 0; i < N; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

template<typename T, unsigned N>
bool equals(const Vec<N, T>& a, const Vec<N, T>& b) {
    for (unsigned i = 0; i < N; ++i) {
        if (!Math::equals(a[i], b[i])) {
            return false;
        }
    }
    return true;
}

template<typename T, unsigned N>
inline T length_sqr(const Vec<N, T>& a) {
    return dot(a, a);
}

template<typename T, unsigned N>
inline T length(const Vec<N, T>& a) {
    return std::sqrt(length_sqr(a));
}

template<typename T>
inline T length_sqr(Vec<2, T> a) {
    return a.x * a.x + a.y * a.y;
}

template<typename T>
inline T length(Vec<2, T> a) {
    return std::sqrt(a.x * a.x + a.y * a.y);
}

template<typename T, unsigned N>
inline T distance(const Vec<N, T>& a, const Vec<N, T>& b) {
    return length(a - b);
}

template<typename T, unsigned N>
inline T distance_sqr(const Vec<N, T>& a, const Vec<N, T>& b) {
    return length_sqr(a - b);
}

template<typename T, unsigned N>
inline Vec<N, T> reflect(const Vec<N, T>& direction, const Vec<N, T>& normal) {
    // factor = -2 * dot(normal, direction)
    // reflected = factor * normal + direction;
    return direction - T{1} * dot(normal, direction) * normal;
}

inline Vec2<float> normalize_2f(Vec2<float> a) {
    return a * Math::Q_rsqrt(a.x * a.x + a.y * a.y);
}

inline Vec3<float> normalize_3f(Vec3<float> a) {
    return a * Math::Q_rsqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

template<unsigned N>
inline Vec<N, float> normalize(const Vec<N, float>& a) {
    return a * Math::Q_rsqrt(dot(a, a));
}

template<typename T, unsigned N>
inline Vec<N, T> normalize(const Vec<N, T>& a) {
    return a / std::sqrt(dot(a, a));
}

template<typename T, unsigned N>
inline Vec<N, T> lerp(const Vec<N, T>& begin, const Vec<N, T>& end, T t) {
    return (T{1} - t) * begin + t * end;
}

// specialized functions
template<typename T>
inline Vec<3, T> cross(const Vec<3, T>& a, const Vec<3, T>& b) {
    return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
    };
}

template<typename T>
inline T sign(const Vec<2, T>& p_1, const Vec<2, T>& p_2, const Vec<2, T>& p_3) {
    return (p_1.x - p_3.x) * (p_2.y - p_3.y) - (p_2.x - p_3.x) * (p_1.y - p_3.y);
}

template<typename T, unsigned N>
constexpr Vec<N, T> min_components(const Vec<N, T>& a, const Vec<N, T>& b) noexcept {
    Vec<N, T> r;
    for (unsigned i = 0; i < N; ++i) {
        r[i] = std::min(a[i], b[i]);
    }
    return r;
}

template<typename T, unsigned N>
constexpr Vec<N, T> max_components(const Vec<N, T>& a, const Vec<N, T>& b) noexcept {
    Vec<N, T> r;
    for (unsigned i = 0; i < N; ++i) {
        r[i] = std::max(a[i], b[i]);
    }
    return r;
}

#undef MAKE_ARRAY_ACCESS

}
