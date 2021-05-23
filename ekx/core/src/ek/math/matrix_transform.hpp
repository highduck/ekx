#pragma once

#include "mat4x4.hpp"
#include "quaternion.hpp"

namespace ek {

template<typename T>
matrix_t<4, 4, T> rotate(const matrix_t<4, 4, T>& m, T angle, const vec3_t <T>& v) {
    T const a = angle;
    T const c = cos(a);
    T const s = sin(a);

    vec3_t<T> axis{normalize(v)};
    vec3_t<T> temp{(T(1) - c) * axis};

    matrix_t<4, 4, T> r;
    r(0, 0) = c + temp[0] * axis[0];
    r(0, 1) = temp[0] * axis[1] + s * axis[2];
    r(0, 2) = temp[0] * axis[2] - s * axis[1];

    r(1, 0) = temp[1] * axis[0] - s * axis[2];
    r(1, 1) = c + temp[1] * axis[1];
    r(1, 2) = temp[1] * axis[2] + s * axis[0];

    r(2, 0) = temp[2] * axis[0] + s * axis[1];
    r(2, 1) = temp[2] * axis[1] - s * axis[0];
    r(2, 2) = c + temp[2] * axis[2];
    return {
            m.col(0) * r(0, 0) + m.col(1) * r(0, 1) + m.col(2) * r(0, 2),
            m.col(0) * r(1, 0) + m.col(1) * r(1, 1) + m.col(2) * r(1, 2),
            m.col(0) * r(2, 0) + m.col(1) * r(2, 1) + m.col(2) * r(2, 2),
            m.col(3)
    };
}

// X-Z-Y
template<typename T>
inline matrix_t<4, 4, T> rotation_transform(const vec3_t <T>& v) {
    T c1 = cos(-v.x);
    T c2 = cos(-v.y);
    T c3 = cos(-v.z);
    T s1 = sin(-v.x);
    T s2 = sin(-v.y);
    T s3 = sin(-v.z);

    matrix_t<4, 4, T> m{};
    m(0, 0) = c2 * c3;
    m(0, 1) = -c1 * s3 + s1 * s2 * c3;
    m(0, 2) = s1 * s3 + c1 * s2 * c3;
    m(0, 3) = static_cast<T>(0);
    m(1, 0) = c2 * s3;
    m(1, 1) = c1 * c3 + s1 * s2 * s3;
    m(1, 2) = -s1 * c3 + c1 * s2 * s3;
    m(1, 3) = static_cast<T>(0);
    m(2, 0) = -s2;
    m(2, 1) = s1 * c2;
    m(2, 2) = c1 * c2;
    m(2, 3) = static_cast<T>(0);
    m(3, 0) = static_cast<T>(0);
    m(3, 1) = static_cast<T>(0);
    m(3, 2) = static_cast<T>(0);
    m(3, 3) = static_cast<T>(1);
    return m;
}

template<typename T>
inline matrix_t<4, 4, T> rotation_transform(const quat_t <T>& q) {
    T qxx = q.x * q.x;
    T qyy = q.y * q.y;
    T qzz = q.z * q.z;
    T qxz = q.x * q.z;
    T qxy = q.x * q.y;
    T qyz = q.y * q.z;
    T qwx = q.w * q.x;
    T qwy = q.w * q.y;
    T qwz = q.w * q.z;

    matrix_t<4, 4, T> m{};
    m(0, 0) = T(1) - T(2) * (qyy + qzz);
    m(0, 1) = T(2) * (qxy + qwz);
    m(0, 2) = T(2) * (qxz - qwy);

    m(1, 0) = T(2) * (qxy - qwz);
    m(1, 1) = T(1) - T(2) * (qxx + qzz);
    m(1, 2) = T(2) * (qyz + qwx);

    m(2, 0) = T(2) * (qxz + qwy);
    m(2, 1) = T(2) * (qyz - qwx);
    m(2, 2) = T(1) - T(2) * (qxx + qyy);
    return m;
}

template<typename T>
matrix_t<4, 4, T> translate_transform(const vec_t<3, T>& translation) {
    matrix_t<4, 4, T> m{};
    m(3, 0) = translation.x;
    m(3, 1) = translation.y;
    m(3, 2) = translation.z;
    return m;
}

template<typename T>
matrix_t<4, 4, T> scale_transform(const vec_t<3, T>& scale) {
    matrix_t<4, 4, T> m{};
    m(0, 0) = scale.x;
    m(1, 1) = scale.y;
    m(2, 2) = scale.z;
    return m;
}

template<typename T>
vec_t<3, T> extract_translation(const matrix_t<4, 4, T>& m) {
    return vec_t<3, T>{m(3, 0), m(3, 1), m(3, 2)};
}

}
