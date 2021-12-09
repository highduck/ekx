#pragma once

#include "Matrix4.hpp"

namespace ek {

template<typename T>
Matrix<2, 2, T> affine_inverse(Matrix<2, 2, T> m) {
    T d = det(m);

    Matrix4<T> r{};
    r(0, 0) = m(1, 1) / d;
    r(0, 1) = m(0, 1) / d;
    r(1, 0) = m(1, 0) / d;
    r(1, 1) = m(0, 0) / d;
    return r;
}

template<typename T>
Matrix<3, 3, T> inverse(Matrix<3, 3, T> m) {
    T det_inv = static_cast<T>(1) / (
            + m(0, 0) * (m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2))
            - m(1, 0) * (m(0, 1) * m(2, 2) - m(2, 1) * m(0, 2))
            + m(2, 0) * (m(0, 1) * m(1, 2) - m(1, 1) * m(0, 2)));

    Matrix<3, 3, T> result{};
    result(0, 0) = + (m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2)) * det_inv;
    result(1, 0) = - (m(1, 0) * m(2, 2) - m(2, 0) * m(1, 2)) * det_inv;
    result(2, 0) = + (m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1)) * det_inv;
    result(0, 1) = - (m(0, 1) * m(2, 2) - m(2, 1) * m(0, 2)) * det_inv;
    result(1, 1) = + (m(0, 0) * m(2, 2) - m(2, 0) * m(0, 2)) * det_inv;
    result(2, 1) = - (m(0, 0) * m(2, 1) - m(2, 0) * m(0, 1)) * det_inv;
    result(0, 2) = + (m(0, 1) * m(1, 2) - m(1, 1) * m(0, 2)) * det_inv;
    result(1, 2) = - (m(0, 0) * m(1, 2) - m(1, 0) * m(0, 2)) * det_inv;
    result(2, 2) = + (m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1)) * det_inv;

    return result;
}

template<typename T>
Matrix<4, 4, T> inverse(Matrix<4, 4, T> m) {

    using vec4 = Vec4<T>;

    T coef_00 = m(2, 2) * m(3, 3) - m(3, 2) * m(2, 3);
    T coef_02 = m(1, 2) * m(3, 3) - m(3, 2) * m(1, 3);
    T coef_03 = m(1, 2) * m(2, 3) - m(2, 2) * m(1, 3);

    T coef_04 = m(2, 1) * m(3, 3) - m(3, 1) * m(2, 3);
    T coef_06 = m(1, 1) * m(3, 3) - m(3, 1) * m(1, 3);
    T coef_07 = m(1, 1) * m(2, 3) - m(2, 1) * m(1, 3);

    T coef_08 = m(2, 1) * m(3, 2) - m(3, 1) * m(2, 2);
    T coef_10 = m(1, 1) * m(3, 2) - m(3, 1) * m(1, 2);
    T coef_11 = m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2);

    T coef_12 = m(2, 0) * m(3, 3) - m(3, 0) * m(2, 3);
    T coef_14 = m(1, 0) * m(3, 3) - m(3, 0) * m(1, 3);
    T coef_15 = m(1, 0) * m(2, 3) - m(2, 0) * m(1, 3);

    T coef_16 = m(2, 0) * m(3, 2) - m(3, 0) * m(2, 2);
    T coef_18 = m(1, 0) * m(3, 2) - m(3, 0) * m(1, 2);
    T coef_19 = m(1, 0) * m(2, 2) - m(2, 0) * m(1, 2);

    T coef_20 = m(2, 0) * m(3, 1) - m(3, 0) * m(2, 1);
    T coef_22 = m(1, 0) * m(3, 1) - m(3, 0) * m(1, 1);
    T coef_23 = m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1);

    vec4 fac_0(coef_00, coef_00, coef_02, coef_03);
    vec4 fac_1(coef_04, coef_04, coef_06, coef_07);
    vec4 fac_2(coef_08, coef_08, coef_10, coef_11);
    vec4 fac_3(coef_12, coef_12, coef_14, coef_15);
    vec4 fac_4(coef_16, coef_16, coef_18, coef_19);
    vec4 fac_5(coef_20, coef_20, coef_22, coef_23);

    vec4 v_0(m(1, 0), m(0, 0), m(0, 0), m(0, 0));
    vec4 v_1(m(1, 1), m(0, 1), m(0, 1), m(0, 1));
    vec4 v_2(m(1, 2), m(0, 2), m(0, 2), m(0, 2));
    vec4 v_3(m(1, 3), m(0, 3), m(0, 3), m(0, 3));

    vec4 inv_0(v_1 * fac_0 - v_2 * fac_1 + v_3 * fac_2);
    vec4 inv_1(v_0 * fac_0 - v_2 * fac_3 + v_3 * fac_4);
    vec4 inv_2(v_0 * fac_1 - v_1 * fac_3 + v_3 * fac_5);
    vec4 inv_3(v_0 * fac_2 - v_1 * fac_4 + v_2 * fac_5);

    vec4 sign_a(+1, -1, +1, -1);
    vec4 sign_b(-1, +1, -1, +1);
    Matrix<4, 4, T> inv_m(inv_0 * sign_a, inv_1 * sign_b, inv_2 * sign_a, inv_3 * sign_b);

    vec4 dot_0(m.col(0) * inv_m.row(0));
    T dot_1 = (dot_0.x + dot_0.y) + (dot_0.z + dot_0.w);

    T inv_det = static_cast<T>(1) / dot_1;

    return inv_m * inv_det;
}

}