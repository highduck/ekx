#pragma once

#include <ek/math/vec.hpp>
#include <ek/math/packed_color.hpp>
#include <ek/math/matrix_fwd.hpp>
#include <ek/math/box.hpp>

namespace ek::static_test {

template<typename T, unsigned N>
struct check_vec_no_padding {
    static_assert(sizeof(vec_t<T, N>) == sizeof(T) * N, "vec_t has extra padding");
};

using f2_ = check_vec_no_padding<float, 2>;
using f3_ = check_vec_no_padding<float, 3>;
using f4_ = check_vec_no_padding<float, 4>;

using d2_ = check_vec_no_padding<double, 2>;
using d3_ = check_vec_no_padding<double, 3>;
using d4_ = check_vec_no_padding<double, 4>;

using u2_ = check_vec_no_padding<uint32_t, 2>;
using u3_ = check_vec_no_padding<uint32_t, 3>;
using u4_ = check_vec_no_padding<uint32_t, 4>;

static_assert(sizeof(premultiplied_abgr32_t) == sizeof(uint8_t) * 4, "extra padding");
static_assert(sizeof(argb32_t) == sizeof(uint8_t) * 4, "extra padding");
static_assert(sizeof(abgr32_t) == sizeof(uint8_t) * 4, "extra padding");

template<typename T, unsigned N, unsigned M>
struct check_mat_no_padding {
    static_assert(sizeof(matrix_t<N, M, T>) == sizeof(T) * N * M, "matrix_t type has extra padding");
};

using mf2x2_ = check_mat_no_padding<float, 2, 2>;
using mf3x2_ = check_mat_no_padding<float, 3, 2>;
using mf4x4_ = check_mat_no_padding<float, 4, 4>;

using md2x2_ = check_mat_no_padding<double, 2, 2>;
using md3x2_ = check_mat_no_padding<double, 3, 2>;
using md4x4_ = check_mat_no_padding<double, 4, 4>;

template<typename T, unsigned N>
struct check_box_no_padding {
    static_assert(sizeof(box_t<T, N>) == sizeof(T) * N * 2, "box_t has extra padding");
};

using b2_f_ = check_box_no_padding<float, 2>;
using b3_f_ = check_box_no_padding<float, 3>;
using b2_d_ = check_box_no_padding<double, 2>;
using b3_d_ = check_box_no_padding<double, 3>;
using b2_u32_ = check_box_no_padding<uint32_t, 2>;
using b3_u32_ = check_box_no_padding<uint32_t, 3>;
using b2_u16_ = check_box_no_padding<uint16_t, 2>;
using b3_u16_ = check_box_no_padding<uint16_t, 3>;
using b2_i32_ = check_box_no_padding<int32_t, 2>;
using b3_i32_ = check_box_no_padding<int32_t, 3>;
using b2_i16_ = check_box_no_padding<int16_t, 2>;
using b3_i16_ = check_box_no_padding<int16_t, 3>;

}