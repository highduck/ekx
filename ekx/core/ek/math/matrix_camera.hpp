#pragma once

#include "mat4x4.hpp"

namespace ek {

// ZO meaning:
// "The near and far clip planes correspond to z normalized device coordinates of 0 and +1 respectively. (Direct3D clip volume definition)"
// from @glm library

// perspectiveRH_ZO
template<typename T>
matrix_t<4, 4, T> perspective_rh(T fov_y, T aspect, T z_near, T z_far) {
    const T tan_half_fov_y = std::tan(fov_y / static_cast<T>(2));
    matrix_t<4, 4, T> m{0};
    m(0, 0) = static_cast<T>(1) / (aspect * tan_half_fov_y);
    m(1, 1) = static_cast<T>(1) / tan_half_fov_y;

    m(2, 2) = z_far / (z_near - z_far);
    m(2, 3) = -static_cast<T>(1);
    m(3, 2) = -(z_far * z_near) / (z_far - z_near);

    return m;
}

// perspectiveLH_ZO
template<typename T>
matrix_t<4, 4, T> perspective_lh(T fov_y, T aspect, T z_near, T z_far) {
    const T tan_half_fov_y = std::tan(fov_y / static_cast<T>(2));
    matrix_t<4, 4, T> m{0};
    m(0, 0) = static_cast<T>(1) / (aspect * tan_half_fov_y);
    m(1, 1) = static_cast<T>(1) / tan_half_fov_y;
    m(2, 2) = z_far / (z_far - z_near);
    m(2, 3) = static_cast<T>(1);
    m(3, 2) = -(z_far * z_near) / (z_far - z_near);
    return m;
}

// orthoLH_ZO
template<typename T>
mat4_t <T> ortho_projection_lh(T left, T right, T bottom, T top, T z_near, T z_far) {
    mat4_t<T> m{};
    m(0, 0) = static_cast<T>(2) / (right - left);
    m(1, 1) = static_cast<T>(2) / (top - bottom);
    m(2, 2) = static_cast<T>(1) / (z_far - z_near);
    m(3, 0) = -(right + left) / (right - left);
    m(3, 1) = -(top + bottom) / (top - bottom);
    m(3, 2) = -z_near / (z_far - z_near);
    return m;
}

// orthoRH_ZO
template<typename T>
mat4_t <T> ortho_projection_rh(T left, T right, T bottom, T top, T z_near, T z_far) {
    mat4_t<T> m{};
    m(0, 0) = static_cast<T>(2) / (right - left);
    m(1, 1) = static_cast<T>(2) / (top - bottom);
    m(2, 2) = -static_cast<T>(1) / (z_far - z_near);

    m(3, 0) = -(right + left) / (right - left);
    m(3, 1) = -(top + bottom) / (top - bottom);
    m(3, 2) = -z_near / (z_far - z_near);
    return m;
}

template<typename T>
mat4_t <T> ortho_2d(T x, T y, T width, T height, T z_near = -1, T z_far = 1) {
    assert(width != 0 && height != 0);
    return ortho_projection_rh(x, x + width, y + height, y, z_near, z_far);
}

// TODO:  look at view matrix calc
template<typename T>
matrix_t<4, 4, T> look_at_rh(const vec_t<3, T>& eye, const vec_t<3, T>& center, const vec_t<3, T>& up) {
    const vec_t<3, T> f(normalize(center - eye));
    const vec_t<3, T> s(normalize(cross(f, up)));
    const vec_t<3, T> u(cross(s, f));

    matrix_t<4, 4, T> result(1);
    result(0, 0) = s.x;
    result(1, 0) = s.y;
    result(2, 0) = s.z;
    result(0, 1) = u.x;
    result(1, 1) = u.y;
    result(2, 1) = u.z;
    result(0, 2) = -f.x;
    result(1, 2) = -f.y;
    result(2, 2) = -f.z;
    result(3, 0) = -dot(s, eye);
    result(3, 1) = -dot(u, eye);
    result(3, 2) = dot(f, eye);
    return result;
}

template<typename T>
matrix_t<4, 4, T> look_at_lh(const vec_t<3, T>& eye, const vec_t<3, T>& center, const vec_t<3, T>& up) {
    const vec_t<3, T> f(normalize(center - eye));
    const vec_t<3, T> s(normalize(cross(up, f)));
    const vec_t<3, T> u(cross(f, s));

    matrix_t<4, 4, T> result(1);
    result(0, 0) = s.x;
    result(1, 0) = s.y;
    result(2, 0) = s.z;
    result(0, 1) = u.x;
    result(1, 1) = u.y;
    result(2, 1) = u.z;
    result(0, 2) = f.x;
    result(1, 2) = f.y;
    result(2, 2) = f.z;
    result(3, 0) = -dot(s, eye);
    result(3, 1) = -dot(u, eye);
    result(3, 2) = -dot(f, eye);
    return result;
}
}