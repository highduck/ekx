#pragma once

namespace ek {

template<typename T, unsigned N>
struct vec_t;

template<typename T>
using vec2_t = vec_t<T, 2u>;
using float2 = vec_t<float, 2u>;
using int2 = vec_t<int, 2u>;

template<typename T>
using vec3_t = vec_t<T, 3u>;
using float3 = vec_t<float, 3u>;
using int3 = vec_t<int, 3u>;

template<typename T>
using vec4_t = vec_t<T, 4u>;
using float4 = vec_t<float, 4u>;
using int4 = vec_t<int, 4u>;

}