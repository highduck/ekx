#pragma once

#include <cmath>
#include "Vec.hpp"
#include "MatrixDecl.hpp"

namespace ek {

template<typename T>
struct Matrix<2, 2, T> {

    using vec2 = Vec2<T>;

#include <ek/math/internal/compiler_unsafe_begin.h>

    union {
        struct {
            T a, b, c, d;
        };
        struct {
            T m00, m01;
            T m10, m11;
        };
        T data_[2][2];
        T m[4];
    };

#include <ek/math/internal/compiler_unsafe_end.h>

    explicit Matrix(T a_ = 1, T b_ = 0,
                    T c_ = 0, T d_ = 1) noexcept
            : a(a_), b(b_),
              c(c_), d(d_) {
    }

    void set_identity() {
        a = 1;
        b = 0;
        c = 0;
        d = 1;
    }

    inline T& operator[](const unsigned index) {
        return m[index];
    }

    inline const T& operator[](const unsigned index) const {
        return m[index];
    }

    inline T& operator()(const unsigned col, const unsigned row) {
        return m[col * 2 + row];
    }

    inline const T& operator()(const unsigned col, const unsigned row) const {
        return m[col * 2 + row];
    }
};

template<typename T>
inline T det(const Matrix<2, 2, T>& m) {
    return m.a * m.d - m.c * m.b;
}

}