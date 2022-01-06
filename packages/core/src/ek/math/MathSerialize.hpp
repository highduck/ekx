#pragma once

#include <ek/serialize/serialize.hpp>

#include <ek/math/ColorTransform.hpp>
#include <ek/math/Color32.hpp>
#include <ek/math/MatrixDecl.hpp>
#include <ek/math/Vec.hpp>
#include <ek/math/Rect.hpp>

namespace ek {

#define DEF_POD(T) template<> struct declared_as_pod_type<T> : public std::true_type {}

DEF_POD(rect_t);
DEF_POD(rect_i16_t);
DEF_POD(recti_t);
DEF_POD(vec2_t);
DEF_POD(vec3_t);
DEF_POD(vec4_t);
DEF_POD(mat3x2_t);
DEF_POD(mat4_t);
DEF_POD(rgba_t);
DEF_POD(circle_t);
DEF_POD(color_mod_t);

DEF_POD(ColorTransformF);
DEF_POD(abgr32_t);
DEF_POD(argb32_t);

#undef DEF_POD

template<unsigned N, unsigned M, typename T>
struct declared_as_pod_type<Matrix<N, M, T>> : public std::true_type {
};

template<typename T, unsigned N>
struct declared_as_pod_type<Rect<N, T>> : public std::true_type {
};

template<typename T, unsigned N>
struct declared_as_pod_type<Vec<N, T>> : public std::true_type {
};

}
