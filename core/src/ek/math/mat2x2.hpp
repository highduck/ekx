#pragma once

#include <cmath>
#include "vec.hpp"
#include "matrix_fwd.hpp"

namespace ek {

template<typename T>
struct matrix_t<2, 2, T> {

    using vec2 = vec2_t<T>;
    using mat2x2 = matrix_t<2, 2, T>;

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

    explicit matrix_t(T a_ = 1, T b_ = 0,
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
inline T det(const matrix_t<T, 2, 2>& m) {
    return m.a * m.d - m.c * m.b;
}

}