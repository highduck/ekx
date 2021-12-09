#pragma once

#include <cmath>
#include "Vec.hpp"
#include "MatrixDecl.hpp"

namespace ek {

template<typename T>
struct Matrix<3, 3, T> {

    using vec3 = Vec3<T>;
    using mat3x3 = Matrix<3, 3, T>;

#include <ek/math/internal/compiler_unsafe_begin.h>

    union {
        struct {
            T m00, m01, m02;
            T m10, m11, m12;
            T m20, m21, m22;
        };
        T data_[3][3];
        T m[9];
    };

#include <ek/math/internal/compiler_unsafe_end.h>

    explicit Matrix() noexcept
            : m00(1), m01(0), m02(0),
              m10(0), m11(1), m12(0),
              m20(0), m21(0), m22(1) {
    }

    explicit Matrix(const Matrix<4, 4, T>& m) noexcept;

    inline Matrix(const vec3& u0, const vec3& u1, const vec3& u2) noexcept
            : m00(u0.x), m01(u0.y), m02(u0.z),
              m10(u1.x), m11(u1.y), m12(u1.z),
              m20(u2.x), m21(u2.y), m22(u2.z) {
    }

    void set_identity() {
        m00 = 1;
        m01 = 0;
        m02 = 0;
        m10 = 0;
        m11 = 1;
        m12 = 0;
        m20 = 0;
        m21 = 0;
        m22 = 1;
    }

    inline T& operator[](const unsigned index) {
        return m[index];
    }

    inline const T& operator[](const unsigned index) const {
        return m[index];
    }

    inline T& operator()(const unsigned col, const unsigned row) {
        return m[col * 3 + row];
    }

    inline const T& operator()(const unsigned col, const unsigned row) const {
        return m[col * 3 + row];
    }
};

template<typename T>
inline T det(const Matrix<3, 3, T>& m) {
    return m.a * m.d - m.c * m.b;
}

}