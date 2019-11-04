#pragma once

#include <cmath>
#include <cassert>
#include "common.hpp"
#include "vec.hpp"
#include "mat3x3.hpp"

namespace ek {

template<typename T>
struct matrix_t<4, 4, T> {

    using vec3 = vec3_t<T>;
    using vec4 = vec4_t<T>;
    using mat4x4 = matrix_t<4, 4, T>;

#include <ek/math/internal/compiler_unsafe_begin.h>

    union {
        struct {
            T m00, m01, m02, m03;
            T m10, m11, m12, m13;
            T m20, m21, m22, m23;
            T m30, m31, m32, m33;
        };
        T data_[4][4];
        T m[16];
    };

#include <ek/math/internal/compiler_unsafe_end.h>

    inline explicit matrix_t(T v = 1) {
        set_identity(v);
    }

    inline matrix_t(T m00_, T m01_, T m02_, T m03_,
                    T m10_, T m11_, T m12_, T m13_,
                    T m20_, T m21_, T m22_, T m23_,
                    T m30_, T m31_, T m32_, T m33_) noexcept
            : m00{m00_}, m01{m01_}, m02{m02_}, m03{m03_},
              m10{m10_}, m11{m11_}, m12{m12_}, m13{m13_},
              m20{m20_}, m21{m21_}, m22{m22_}, m23{m23_},
              m30{m30_}, m31{m31_}, m32{m32_}, m33{m33_} {

    }

    inline matrix_t(const vec4& v0, const vec4& v1, const vec4& v2, const vec4& v3) noexcept {
        (*this)(0, 0) = v0.x;
        (*this)(0, 1) = v0.y;
        (*this)(0, 2) = v0.z;
        (*this)(0, 3) = v0.w;
        (*this)(1, 0) = v1.x;
        (*this)(1, 1) = v1.y;
        (*this)(1, 2) = v1.z;
        (*this)(1, 3) = v1.w;
        (*this)(2, 0) = v2.x;
        (*this)(2, 1) = v2.y;
        (*this)(2, 2) = v2.z;
        (*this)(2, 3) = v2.w;
        (*this)(3, 0) = v3.x;
        (*this)(3, 1) = v3.y;
        (*this)(3, 2) = v3.z;
        (*this)(3, 3) = v3.w;
    }

    inline matrix_t(const mat4x4& matrix) noexcept {
        for (int i = 0; i < 16; ++i) {
            m[i] = matrix.m[i];
        }
    }

    inline matrix_t(mat4x4&& matrix) noexcept = default;

    mat4x4& operator=(const mat4x4&) noexcept = default;

    mat4x4& operator=(mat4x4&&) noexcept = default;

    inline vec4 col(unsigned i) const {
        return {
                m[i * 4 + 0],
                m[i * 4 + 1],
                m[i * 4 + 2],
                m[i * 4 + 3]
        };
    }

    inline vec4 row(unsigned i) const {
        return {
                m[4 * 0 + i],
                m[4 * 1 + i],
                m[4 * 2 + i],
                m[4 * 3 + i]
        };
    }

    void set_identity(T v = 1) {
        for (T& value : m) {
            value = 0;
        }
        m00 = v;
        m11 = v;
        m22 = v;
        m33 = v;
    }

    inline T& operator[](const unsigned index) {
        return m[index];
    }

    inline const T& operator[](const unsigned index) const {
        return m[index];
    }

    inline T& operator()(const unsigned col, const unsigned row) {
        return m[col * 4 + row];
    }

    inline const T& operator()(const unsigned col, const unsigned row) const {
        return m[col * 4 + row];
    }

    void transform_2d(T x, T y, T scale = 1, T rotation_degrees = 0) {
        T rotation = math::to_radians(rotation_degrees);
        T cs = std::cos(rotation) * scale;
        T sn = std::sin(rotation) * scale;

        set_identity();
        m[0] = cs;
        m[1] = -sn;
        m[4] = sn;
        m[5] = cs;
        m[12] = x;
        m[13] = y;
    }

    inline mat4x4& operator*=(T scalar) {
        *this = *this * scalar;
        return *this;
    }

    inline mat4x4& operator/=(T scalar) {
        *this = *this / scalar;
        return *this;
    }

    inline mat4x4& operator+=(const mat4x4& v) {
        *this = *this + v;
        return *this;
    }

    inline mat4x4& operator-=(const mat4x4& v) {
        *this = *this - v;
        return *this;
    }
};

template<typename T>
matrix_t<4, 4, T> operator*(const matrix_t<4, 4, T>& m, const T f) {
    matrix_t<4, 4, T> r;
    for (int i = 0; i < 16; ++i) {
        r.m[i] = m.m[i] * f;
    }
    return r;
}

template<typename T>
matrix_t<4, 4, T> operator/(const matrix_t<4, 4, T>& m, const T f) {
    matrix_t<4, 4, T> r;
    for (int i = 0; i < 16; ++i) {
        r.m[i] = m.m[i] / f;
    }
    return r;
}

template<typename T>
matrix_t<4, 4, T> operator+(const matrix_t<4, 4, T>& m1, const matrix_t<4, 4, T>& m2) {
    matrix_t<4, 4, T> r;
    for (int i = 0; i < 16; ++i) {
        r.m[i] = m1.m[i] + m2.m[i];
    }
    return r;
}

template<typename T>
inline matrix_t<4, 4, T> operator*(const matrix_t<4, 4, T>& l, const matrix_t<4, 4, T>& r) {
    return matrix_t<4, 4, T>{
            r.m00 * l.m00 + r.m01 * l.m10 + r.m02 * l.m20 + r.m03 * l.m30,
            r.m00 * l.m01 + r.m01 * l.m11 + r.m02 * l.m21 + r.m03 * l.m31,
            r.m00 * l.m02 + r.m01 * l.m12 + r.m02 * l.m22 + r.m03 * l.m32,
            r.m00 * l.m03 + r.m01 * l.m13 + r.m02 * l.m23 + r.m03 * l.m33,

            r.m10 * l.m00 + r.m11 * l.m10 + r.m12 * l.m20 + r.m13 * l.m30,
            r.m10 * l.m01 + r.m11 * l.m11 + r.m12 * l.m21 + r.m13 * l.m31,
            r.m10 * l.m02 + r.m11 * l.m12 + r.m12 * l.m22 + r.m13 * l.m32,
            r.m10 * l.m03 + r.m11 * l.m13 + r.m12 * l.m23 + r.m13 * l.m33,

            r.m20 * l.m00 + r.m21 * l.m10 + r.m22 * l.m20 + r.m23 * l.m30,
            r.m20 * l.m01 + r.m21 * l.m11 + r.m22 * l.m21 + r.m23 * l.m31,
            r.m20 * l.m02 + r.m21 * l.m12 + r.m22 * l.m22 + r.m23 * l.m32,
            r.m20 * l.m03 + r.m21 * l.m13 + r.m22 * l.m23 + r.m23 * l.m33,

            r.m30 * l.m00 + r.m31 * l.m10 + r.m32 * l.m20 + r.m33 * l.m30,
            r.m30 * l.m01 + r.m31 * l.m11 + r.m32 * l.m21 + r.m33 * l.m31,
            r.m30 * l.m02 + r.m31 * l.m12 + r.m32 * l.m22 + r.m33 * l.m32,
            r.m30 * l.m03 + r.m31 * l.m13 + r.m32 * l.m23 + r.m33 * l.m33
    };
}

//! Post-multiply.
template<typename T>
vec_t<T, 4>
operator*(const matrix_t<4, 4, T>& m, const vec_t<T, 4>& v) {
    return vec_t<T, 4>(v.x * m.m00 + v.y * m.m01 + v.z * m.m02 + v.w * m.m03,
                       v.x * m.m10 + v.y * m.m11 + v.z * m.m12 + v.w * m.m13,
                       v.x * m.m20 + v.y * m.m21 + v.z * m.m22 + v.w * m.m23,
                       v.x * m.m30 + v.y * m.m31 + v.z * m.m32 + v.w * m.m33);
}

template<typename T>
vec_t<T, 3>
operator*(const matrix_t<4, 4, T>& m, const vec_t<T, 3>& v) {
    const float w = v.x * m.m30 + v.y * m.m31 + v.z * m.m32 + m.m33;
    return vec_t<T, 3>(v.x * m.m00 + v.y * m.m01 + v.z * m.m02 + m.m03,
                       v.x * m.m10 + v.y * m.m11 + v.z * m.m12 + m.m13,
                       v.x * m.m20 + v.y * m.m21 + v.z * m.m22 + m.m23);
}

//! Pre-multiply.
template<typename T>
vec_t<T, 4> operator*(const vec_t<T, 4>& v, const matrix_t<4, 4, T>& m) {
    return vec_t<T, 4>(v.x * m.m00 + v.y * m.m10 + v.z * m.m20 + v.w * m.m30,
                       v.x * m.m01 + v.y * m.m11 + v.z * m.m21 + v.w * m.m31,
                       v.x * m.m02 + v.y * m.m12 + v.z * m.m22 + v.w * m.m32,
                       v.x * m.m03 + v.y * m.m13 + v.z * m.m23 + v.w * m.m33);
}

template<typename T>
vec_t<T, 3> operator*(const vec_t<T, 3>& v, const matrix_t<4, 4, T>& m) {
    return vec_t<T, 3>(v.x * m.m00 + v.y * m.m10 + v.z * m.m20 + m.m30,
                       v.x * m.m01 + v.y * m.m11 + v.z * m.m21 + m.m31,
                       v.x * m.m02 + v.y * m.m12 + v.z * m.m22 + m.m32);
}

template<typename T>
matrix_t<3, 3, T>::matrix_t(const matrix_t<4, 4, T>& m) noexcept
        :m00{m.m00}, m01{m.m01}, m02{m.m02},
         m10{m.m10}, m11{m.m11}, m12{m.m12},
         m20{m.m20}, m21{m.m21}, m22{m.m22} {

}
}