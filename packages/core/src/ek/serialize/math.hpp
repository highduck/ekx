#pragma once

#include "core.hpp"
#include <ek/math.h>

namespace ek {

#define DEF_POD(T) template<> struct declared_as_pod_type<T> : public std::true_type {}

DEF_POD(rect_t);
DEF_POD(i16rect_t);
DEF_POD(irect_t);
DEF_POD(aabb2_t);
DEF_POD(aabb3_t);
DEF_POD(ivec2_t);
DEF_POD(ivec3_t);
DEF_POD(ivec4_t);
DEF_POD(vec2_t);
DEF_POD(vec3_t);
DEF_POD(vec4_t);
DEF_POD(quat_t);
DEF_POD(mat2_t);
DEF_POD(mat3x2_t);
DEF_POD(mat3_t);
DEF_POD(mat4_t);
DEF_POD(color_t);
DEF_POD(color2_t);
DEF_POD(color2f_t);

#undef DEF_POD

}
