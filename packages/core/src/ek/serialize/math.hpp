#pragma once

#include "core.hpp"
#include <ek/math.h>

namespace ek {

#define DEF_POD(T) template<> struct declared_as_pod_type<T> : public std::true_type {}

DEF_POD(rect_t);
DEF_POD(rect_i16_t);
DEF_POD(recti_t);
DEF_POD(brect_t);
DEF_POD(aabb_t);
DEF_POD(vec2i_t);
DEF_POD(vec3i_t);
DEF_POD(vec4i_t);
DEF_POD(vec2_t);
DEF_POD(vec3_t);
DEF_POD(vec4_t);
DEF_POD(quat_t);
DEF_POD(mat2_t);
DEF_POD(mat3x2_t);
DEF_POD(mat3_t);
DEF_POD(mat4_t);
DEF_POD(rgba_t);
DEF_POD(color2_t);
DEF_POD(color2f_t);

#undef DEF_POD

}
