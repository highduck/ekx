#pragma once

#include "vec.hpp"

namespace ek {

template<typename T>
struct quat_t {
    using self_type = quat_t<T>;

#include <ek/math/internal/compiler_unsafe_begin.h>

    union {
        struct {
            T x, y, z, w;
        };
        T data_[4];
    };

#include <ek/math/internal/compiler_unsafe_end.h>

    quat_t() noexcept
            : x{0},
              y{0},
              z{0},
              w{1} {

    }

    quat_t(T w_, T x_, T y_, T z_) noexcept
            : x{x_},
              y{y_},
              z{z_},
              w{w_} {

    }

    explicit quat_t(const vec3_t <T>& euler_angles) {
        const vec3_t<T> c{
                cos(euler_angles.x * static_cast<T>(0.5)),
                cos(euler_angles.y * static_cast<T>(0.5)),
                cos(euler_angles.z * static_cast<T>(0.5))
        };
        const vec3_t<T> s{
                sin(euler_angles.x * static_cast<T>(0.5)),
                sin(euler_angles.y * static_cast<T>(0.5)),
                sin(euler_angles.z * static_cast<T>(0.5))
        };

        w = c.x * c.y * c.z + s.x * s.y * s.z;
        x = s.x * c.y * c.z - c.x * s.y * s.z;
        y = c.x * s.y * c.z + s.x * c.y * s.z;
        z = c.x * c.y * s.z - s.x * s.y * c.z;
    }

    quat_t(T w_, const vec_t<3, T>& v)
            : w{w},
              x{v.x}, y{v.y}, z{v.z} {
    }

    explicit quat_t(const matrix_t<4, 4, T>& m) {
        const T four_x_squared_minus_1 = m(0, 0) - m(1, 1) - m(2, 2);
        const T four_y_squared_minus_1 = m(1, 1) - m(0, 0) - m(2, 2);
        const T four_z_squared_minus_1 = m(2, 2) - m(0, 0) - m(1, 1);
        const T four_w_squared_minus_1 = m(0, 0) + m(1, 1) + m(2, 2);

        int biggest_index = 0;
        T four_biggest_squared_minus_1 = four_w_squared_minus_1;
        if (four_x_squared_minus_1 > four_biggest_squared_minus_1) {
            four_biggest_squared_minus_1 = four_x_squared_minus_1;
            biggest_index = 1;
        }
        if (four_y_squared_minus_1 > four_biggest_squared_minus_1) {
            four_biggest_squared_minus_1 = four_y_squared_minus_1;
            biggest_index = 2;
        }
        if (four_z_squared_minus_1 > four_biggest_squared_minus_1) {
            four_biggest_squared_minus_1 = four_z_squared_minus_1;
            biggest_index = 3;
        }

        const T biggest_val = sqrt(four_biggest_squared_minus_1 + static_cast<T>(1)) * static_cast<T>(0.5);
        const T mult = static_cast<T>(0.25) / biggest_val;

        switch (biggest_index) {
            case 0:
                w = biggest_val;
                x = (m(1, 2) - m(2, 1)) * mult;
                y = (m(2, 0) - m(0, 2)) * mult;
                z = (m(0, 1) - m(1, 0)) * mult;
                break;
            case 1:
                w = (m(1, 2) - m(2, 1)) * mult;
                x = biggest_val;
                y = (m(0, 1) + m(1, 0)) * mult;
                z = (m(2, 0) + m(0, 2)) * mult;
                break;
            case 2:
                w = (m(2, 0) - m(0, 2)) * mult;
                x = (m(0, 1) + m(1, 0)) * mult;
                y = biggest_val;
                z = (m(1, 2) + m(2, 1)) * mult;
                break;
            case 3:
                w = (m(0, 1) - m(1, 0)) * mult;
                x = (m(2, 0) + m(0, 2)) * mult;
                y = (m(1, 2) + m(2, 1)) * mult;
                z = biggest_val;
                break;
            default: // Silence a -Wswitch-default warning in GCC. Should never actually get here. Assert is just for sanity.
                assert(false);
                w = 1;
                x = 0;
                y = 0;
                z = 0;
                break;
        }
    }

    inline self_type operator-() const { return self_type(-w, -x, -y, -z); }

    inline self_type operator-(const self_type& v) const { return self_type(w - v.w, x - v.x, y - v.y, z - v.z); }

    inline self_type operator+(const self_type& v) const { return self_type(w + v.w, x + v.x, y + v.y, z + v.z); }

    inline self_type operator*(const self_type& q) const {
        return self_type{w * q.w - x * q.x - y * q.y - z * q.z,
                         w * q.x + x * q.w + y * q.z - z * q.y,
                         w * q.y + y * q.w + z * q.x - x * q.z,
                         w * q.z + z * q.w + x * q.y - y * q.x};
    }

    //inline self_type operator/(const self_type& v) const { return self_type(w / v.w, x / v.x, y / v.y, z / v.z); }

    inline self_type operator*(T scalar) const {
        return self_type(w * scalar, x * scalar, y * scalar, z * scalar);
    }

    inline vec_t<3, T> operator*(const vec_t<3, T>& v) const {
        const vec_t<3, T> qua_vector{x, y, z};
        const vec_t<3, T> uv(cross(qua_vector, v));
        const vec_t<3, T> uuv(cross(qua_vector, uv));

        return v + ((uv * w) + uuv) * static_cast<T>(2);
    }

    inline self_type operator/(T scalar) const {
        T inv = T{1} / scalar;
        return self_type(w * inv, x * inv, y * inv, z * inv);
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

//    inline self_type& operator/=(const self_type& v) {
//        *this = *this / v;
//        return *this;
//    }

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

template<typename T>
T roll(const quat_t<T>& q) {
    return static_cast<T>(atan2(static_cast<T>(2) * (q.x * q.y + q.w * q.z),
                                q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z));
}

template<typename T>
T pitch(const quat_t<T>& q) {
    //return T(atan(T(2) * (q.y * q.z + q.w * q.x), q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z));
    const T y = static_cast<T>(2) * (q.y * q.z + q.w * q.x);
    const T x = q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z;

    //avoid atan2(0,0) - handle singularity - Matiis
    if (math::equals(x, 0.0f) && math::equals(y, 0.0f)) {
        return static_cast<T>(static_cast<T>(2) * atan2(q.x, q.w));
    }

    return static_cast<T>(atan2(y, x));
}

template<typename T>
T yaw(const quat_t<T>& q) {
    return asin(math::clamp(static_cast<T>(-2) * (q.x * q.z - q.w * q.y), static_cast<T>(-1), static_cast<T>(1)));
}

template<typename T>
vec3_t <T> euler_angles(const quat_t<T>& q) {
    return {pitch(q), yaw(q), roll(q)};
}

template<typename T>
inline quat_t<T> normalize(const quat_t<T>& a) {
    const T d = sqrt(a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w);
    if (d < math::epsilon<T>()) {
        return {};
    }
    return a * (T{1} / d);
}

template<typename T>
quat_t<T> quat_cast(const matrix_t<3, 3, T>& m) {
    T four_x_squared_minus_1 = m(0, 0) - m(1, 1) - m(2, 2);
    T four_y_squared_minus_1 = m(1, 1) - m(0, 0) - m(2, 2);
    T four_z_squared_minus_1 = m(2, 2) - m(0, 0) - m(1, 1);
    T four_w_squared_minus_1 = m(0, 0) + m(1, 1) + m(2, 2);

    int biggest_index = 0;
    T four_biggest_squared_minus_1 = four_w_squared_minus_1;
    if (four_x_squared_minus_1 > four_biggest_squared_minus_1) {
        four_biggest_squared_minus_1 = four_x_squared_minus_1;
        biggest_index = 1;
    }
    if (four_y_squared_minus_1 > four_biggest_squared_minus_1) {
        four_biggest_squared_minus_1 = four_y_squared_minus_1;
        biggest_index = 2;
    }
    if (four_z_squared_minus_1 > four_biggest_squared_minus_1) {
        four_biggest_squared_minus_1 = four_z_squared_minus_1;
        biggest_index = 3;
    }

    T biggest_val = sqrt(four_biggest_squared_minus_1 + static_cast<T>(1)) * static_cast<T>(0.5);
    T mult = static_cast<T>(0.25) / biggest_val;

    switch (biggest_index) {
        case 0:
            return quat_t<T>(biggest_val,
                             (m(1, 2) - m(2, 1)) * mult,
                             (m(2, 0) - m(0, 2)) * mult,
                             (m(0, 1) - m(1, 0)) * mult);
        case 1:
            return quat_t<T>((m(1, 2) - m(2, 1)) * mult, biggest_val,
                             (m(0, 1) + m(1, 0)) * mult,
                             (m(2, 0) + m(0, 2)) * mult);
        case 2:
            return quat_t<T>((m(2, 0) - m(0, 2)) * mult,
                             (m(0, 1) + m(1, 0)) * mult, biggest_val,
                             (m(1, 2) + m(2, 1)) * mult);
        case 3:
            return quat_t<T>((m(0, 1) - m(1, 0)) * mult,
                             (m(2, 0) + m(0, 2)) * mult,
                             (m(1, 2) + m(2, 1)) * mult,
                             biggest_val);
        default:
            // Silence a -Wswitch-default warning in GCC. Should never actually get here. Assert is just for sanity.
            assert(false);
            return quat_t<T>{};
    }
}

template<typename T>
quat_t<float> quat_between(const vec3_t <T>& u, const vec3_t <T>& v) {
    T norm_u_norm_v = sqrt(dot(u, u) * dot(v, v));
    T real_part = norm_u_norm_v + dot(u, v);
    vec3_t<T> t;

    if (real_part < static_cast<T>(1.e-6f) * norm_u_norm_v) {
        // If u and v are exactly opposite, rotate 180 degrees
        // around an arbitrary orthogonal axis. Axis normalisation
        // can happen later, when we normalise the quaternion.
        real_part = static_cast<T>(0);
        t = abs(u.x) > abs(u.z) ? vec3_t<T>(-u.y, u.x, static_cast<T>(0)) : vec3_t<T>(static_cast<T>(0), -u.z, u.y);
    } else {
        // Otherwise, build quaternion the standard way.
        t = cross(u, v);
    }

    return normalize(quat_t<float>(real_part, t.x, t.y, t.z));
}

template<typename T>
quat_t<T> quat_look_at_rh(const vec_t<3, T>& direction, const vec_t<3, T>& up) {
    vec_t<3, T> u2 = -direction;
    vec_t<3, T> u0 = normalize(cross(up, u2));
    vec_t<3, T> u1 = cross(u2, u0);

    matrix_t<3, 3, T> m{u0, u1, u2};

    return quat_cast(m);
}

}