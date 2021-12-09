#pragma once

#include <cmath>
#include "Vec.hpp"
#include "MatrixDecl.hpp"

namespace ek {

template<typename T>
struct Matrix<3, 2, T> {

    using vec2 = Vec2<T>;
    using Matrix3x2 = Matrix<3, 2, T>;

#include <ek/math/internal/compiler_unsafe_begin.h>

    union {
        struct {
            T a, b, c, d, tx, ty;
//            T a, b, tx, ty;
//            T c, d;
//            T
        };
        struct {
            T m00, m01, m02;
            T m10, m11, m12;
        };
        T data_[2][3];
    };

//    tx = m20
//    ty = m21
//    c = m10
//    d = m11
//    a = m00
//    b = m01

#include <ek/math/internal/compiler_unsafe_end.h>

    explicit Matrix(T a_ = 1, T b_ = 0,
                    T c_ = 0, T d_ = 1,
                    T tx_ = 0, T ty_ = 0) noexcept
            : a(a_), b(b_),
              c(c_), d(d_),
              tx(tx_), ty(ty_) {
    }

    explicit Matrix(const vec2& pos) noexcept
            : a(1), b(0),
              c(0), d(1),
              tx(pos.x), ty(pos.y) {
    }

    // equals to `(self * matrix(1,0,0,1,x,y)).position`
    vec2 transform(T x, T y) const {
        return vec2{
                x * a + y * c + tx,
                x * b + y * d + ty
        };
    }

    inline vec2 transform(vec2 vec) const {
        return transform(vec.x, vec.y);
    }

    void set_identity() {
        a = 1;
        b = 0;
        c = 0;
        d = 1;
        tx = 0;
        ty = 0;
    }

    Matrix3x2& translate(T x, T y) {
        tx += a * x + c * y;
        ty += d * y + b * x;
        return *this;
    }

    Matrix3x2& translate(const vec2& t) {
        tx += a * t.x + c * t.y;
        ty += d * t.y + b * t.x;
        return *this;
    }

    Matrix3x2& scale(T sx, T sy) {
        a *= sx;
        b *= sx;
        c *= sy;
        d *= sy;
        return *this;
    }

    Matrix3x2& scale(const vec2& s) {
        return scale(s.x, s.y);
    }

    vec2 position() const {
        return {tx, ty};
    }

    void position(const vec2& pos) {
        tx = pos.x;
        ty = pos.y;
    }

    void position(T x, T y) {
        tx = x;
        ty = y;
    }

    vec2 scale() const {
        vec2 s{
                std::sqrt(a * a + b * b),
                std::sqrt(c * c + d * d)
        };
//        if (a < 0.0f) s.x = -s.x;
//        if (d < 0.0f) s.y = -s.y;
        return s;
    }

    vec2 skew() const {
        return {
                atan2f(-c, d),
                atan2f(b, a),
        };
    }

    T rotation() const {
        T sx = atan2f(-c, d);
        T sy = atan2f(b, a);
        return sx == sy ? sy : 0;
    }

    Matrix3x2& rotate(T radians) {
        T sn = sinf(radians);
        T cs = cosf(radians);
        T pa = a;
        T pb = b;
        T pc = c;
        T pd = d;
        a = pa * cs + pc * sn;
        c = -pa * sn + pc * cs;
        b = pd * sn + pb * cs;
        d = pd * cs - pb * sn;
        return *this;
    }

    // NOTES: to do `concat` style method - just swap right and left matrices,
    // it will act like `this` is right, `m` is left (premultiplication)
    Matrix3x2 operator*(const Matrix3x2& right) const {
        //return multiply(right);
        Matrix3x2 m;
        m.a = a * right.a + c * right.b;
        m.b = b * right.a + d * right.b;
        m.c = a * right.c + c * right.d;
        m.d = b * right.c + d * right.d;
        m.tx = a * right.tx + c * right.ty + tx;
        m.ty = b * right.tx + d * right.ty + ty;
        return m;

//        r.m00 * l.m00 + r.m01 * l.m10
//        r.m00 * l.m01 + r.m01 * l.m11
//        r.m00 * l.m02 + r.m01 * l.m12
//        r.m00 * l.m03 + r.m01 * l.m13
//        r.m10 * l.m00 + r.m11 * l.m10
//        r.m10 * l.m01 + r.m11 * l.m11
    }

    void apply_transform(const vec2& scale, T rotation) {
        const T sn = sinf(rotation);
        const T cs = cosf(rotation);
        a = cs * scale.x;
        b = sn * scale.x;
        c = -sn * scale.y;
        d = cs * scale.y;
    }

    void set(const vec2& scale, const vec2& skew) {
        a = cosf(skew.y) * scale.x;
        b = sinf(skew.y) * scale.x;
        c = -sinf(skew.x) * scale.y;
        d = cosf(skew.x) * scale.y;
    }

    void set(const vec2& position, const vec2& scale, const vec2& skew) {
        tx = position.x;
        ty = position.y;
        set(scale, skew);
    }

    void transform_inverse(vec2 pos, vec2& out) const {
        const T a_ = a;
        const T b_ = b;
        const T c_ = c;
        const T d_ = d;
        const T x_ = pos.x - tx;
        const T y_ = pos.y - ty;
        const T invDet = T{1} / (a_ * d_ - c_ * b_);
        out.x = (x_ * d_ - y_ * c_) * invDet;
        out.y = (y_ * a_ - x_ * b_) * invDet;
    }

    inline vec2 transformInverse(vec2 pos) const {
        Vec2f result;
        transform_inverse(pos, result);
        return result;
    }

    bool inverse() {
        const T D = det(*this);
        if (D == T{0}) {
            return false;
        }
        const T inv = T{1} / D;
        const float a_ = a * inv;
        const float b_ = b * inv;
        const float c_ = c * inv;
        const float d_ = d * inv;
        const float x = tx;
        const float y = ty;
        a = d_;
        b = -b_;
        c = -c_;
        d = a_;
        tx = y * c_ - x * d_;
        ty = x * b_ - y * a_;

        return true;
    }

    inline static void multiply(const Matrix3x2& l, const Matrix3x2& r, Matrix3x2& out) {
        out.a = l.a * r.a + l.c * r.b;
        out.b = l.b * r.a + l.d * r.b;
        out.c = l.a * r.c + l.c * r.d;
        out.d = l.b * r.c + l.d * r.d;
        out.tx = l.a * r.tx + l.c * r.ty + l.tx;
        out.ty = l.b * r.tx + l.d * r.ty + l.ty;
    }
};

template<typename T>
inline T det(const Matrix<3, 2, T>& m) {
    return m.a * m.d - m.c * m.b;
}

}