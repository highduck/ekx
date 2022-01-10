
#ifndef MATH_VEC_TEST_H
#define MATH_VEC_TEST_H

#include "math_test_common.h"

TEST_SUITE_BEGIN("vec2");

TEST_CASE("vec2_neg") {
vec2_t a = vec2(-11.751342f ,-24.827091f);
vec2_t b = vec2(0.596904f ,6.656758f);
    vec2_t r = neg_vec2(a);
   CHECK_NEAR_EQ(r.x, 11.751342f);
   CHECK_NEAR_EQ(r.y, 24.827091f);
}

TEST_CASE("vec2_add") {
vec2_t a = vec2(-11.751342f ,-24.827091f);
vec2_t b = vec2(0.596904f ,6.656758f);
    vec2_t r = add_vec2(a, b);
   CHECK_NEAR_EQ(r.x, -11.154438f);
   CHECK_NEAR_EQ(r.y, -18.170333f);
}

TEST_CASE("vec2_sub") {
vec2_t a = vec2(-11.751342f ,-24.827091f);
vec2_t b = vec2(0.596904f ,6.656758f);
    vec2_t r = sub_vec2(a, b);
   CHECK_NEAR_EQ(r.x, -12.348246f);
   CHECK_NEAR_EQ(r.y, -31.483849f);
}

TEST_CASE("vec2_mul") {
vec2_t a = vec2(-11.751342f ,-24.827091f);
vec2_t b = vec2(0.596904f ,6.656758f);
    vec2_t r = mul_vec2(a, b);
   CHECK_NEAR_EQ(r.x, -7.014423f);
   CHECK_NEAR_EQ(r.y, -165.267937f);
}

TEST_CASE("vec2_scale") {
vec2_t a = vec2(-11.751342f ,-24.827091f);
vec2_t b = vec2(0.596904f ,6.656758f);
    vec2_t r = scale_vec2(a, b[0]);
   CHECK_NEAR_EQ(r.x, -7.014423f);
   CHECK_NEAR_EQ(r.y, -14.81939f);
}

TEST_CASE("float * vec2") {
vec2_t a = vec2(-11.751342f ,-24.827091f);
vec2_t b = vec2(0.596904f ,6.656758f);
    vec2_t r = a[0] * b;
   CHECK_NEAR_EQ(r.x, -7.014423f);
   CHECK_NEAR_EQ(r.y, -78.22584f);
}

TEST_SUITE_END();
TEST_SUITE_BEGIN("vec3");

TEST_CASE("vec3_neg") {
vec3_t a = vec3(10.75596f ,49.526611f ,-41.090047f);
vec3_t b = vec3(-31.371282f ,37.511148f ,-4.657811f);
    vec3_t r = neg_vec3(a);
   CHECK_NEAR_EQ(r.x, -10.75596f);
   CHECK_NEAR_EQ(r.y, -49.526611f);
   CHECK_NEAR_EQ(r.z, 41.090047f);
}

TEST_CASE("vec3_add") {
vec3_t a = vec3(10.75596f ,49.526611f ,-41.090047f);
vec3_t b = vec3(-31.371282f ,37.511148f ,-4.657811f);
    vec3_t r = add_vec3(a, b);
   CHECK_NEAR_EQ(r.x, -20.615322f);
   CHECK_NEAR_EQ(r.y, 87.037759f);
   CHECK_NEAR_EQ(r.z, -45.747858f);
}

TEST_CASE("vec3_sub") {
vec3_t a = vec3(10.75596f ,49.526611f ,-41.090047f);
vec3_t b = vec3(-31.371282f ,37.511148f ,-4.657811f);
    vec3_t r = sub_vec3(a, b);
   CHECK_NEAR_EQ(r.x, 42.127242f);
   CHECK_NEAR_EQ(r.y, 12.015463f);
   CHECK_NEAR_EQ(r.z, -36.432236f);
}

TEST_CASE("vec3_mul") {
vec3_t a = vec3(10.75596f ,49.526611f ,-41.090047f);
vec3_t b = vec3(-31.371282f ,37.511148f ,-4.657811f);
    vec3_t r = mul_vec3(a, b);
   CHECK_NEAR_EQ(r.x, -337.428254f);
   CHECK_NEAR_EQ(r.y, 1857.800035f);
   CHECK_NEAR_EQ(r.z, 191.389673f);
}

TEST_CASE("vec3_scale") {
vec3_t a = vec3(10.75596f ,49.526611f ,-41.090047f);
vec3_t b = vec3(-31.371282f ,37.511148f ,-4.657811f);
    vec3_t r = scale_vec3(a, b[0]);
   CHECK_NEAR_EQ(r.x, -337.428254f);
   CHECK_NEAR_EQ(r.y, -1553.71328f);
   CHECK_NEAR_EQ(r.z, 1289.047452f);
}

TEST_CASE("float * vec3") {
vec3_t a = vec3(10.75596f ,49.526611f ,-41.090047f);
vec3_t b = vec3(-31.371282f ,37.511148f ,-4.657811f);
    vec3_t r = a[0] * b;
   CHECK_NEAR_EQ(r.x, -337.428254f);
   CHECK_NEAR_EQ(r.y, 403.468407f);
   CHECK_NEAR_EQ(r.z, -50.099229f);
}

TEST_SUITE_END();
TEST_SUITE_BEGIN("vec4");

TEST_CASE("vec4_neg") {
vec4_t a = vec4(-47.124798f ,48.365108f ,-2.354816f ,-17.838881f);
vec4_t b = vec4(16.553662f ,-28.483541f ,-47.156044f ,-47.322734f);
    vec4_t r = neg_vec4(a);
   CHECK_NEAR_EQ(r.x, 47.124798f);
   CHECK_NEAR_EQ(r.y, -48.365108f);
   CHECK_NEAR_EQ(r.z, 2.354816f);
   CHECK_NEAR_EQ(r.w, 17.838881f);
}

TEST_CASE("vec4_add") {
vec4_t a = vec4(-47.124798f ,48.365108f ,-2.354816f ,-17.838881f);
vec4_t b = vec4(16.553662f ,-28.483541f ,-47.156044f ,-47.322734f);
    vec4_t r = add_vec4(a, b);
   CHECK_NEAR_EQ(r.x, -30.571136f);
   CHECK_NEAR_EQ(r.y, 19.881567f);
   CHECK_NEAR_EQ(r.z, -49.51086f);
   CHECK_NEAR_EQ(r.w, -65.161615f);
}

TEST_CASE("vec4_sub") {
vec4_t a = vec4(-47.124798f ,48.365108f ,-2.354816f ,-17.838881f);
vec4_t b = vec4(16.553662f ,-28.483541f ,-47.156044f ,-47.322734f);
    vec4_t r = sub_vec4(a, b);
   CHECK_NEAR_EQ(r.x, -63.67846f);
   CHECK_NEAR_EQ(r.y, 76.848649f);
   CHECK_NEAR_EQ(r.z, 44.801228f);
   CHECK_NEAR_EQ(r.w, 29.483853f);
}

TEST_CASE("vec4_mul") {
vec4_t a = vec4(-47.124798f ,48.365108f ,-2.354816f ,-17.838881f);
vec4_t b = vec4(16.553662f ,-28.483541f ,-47.156044f ,-47.322734f);
    vec4_t r = mul_vec4(a, b);
   CHECK_NEAR_EQ(r.x, -780.087978f);
   CHECK_NEAR_EQ(r.y, -1377.609537f);
   CHECK_NEAR_EQ(r.z, 111.043807f);
   CHECK_NEAR_EQ(r.w, 844.18462f);
}

TEST_CASE("vec4_scale") {
vec4_t a = vec4(-47.124798f ,48.365108f ,-2.354816f ,-17.838881f);
vec4_t b = vec4(16.553662f ,-28.483541f ,-47.156044f ,-47.322734f);
    vec4_t r = scale_vec4(a, b[0]);
   CHECK_NEAR_EQ(r.x, -780.087978f);
   CHECK_NEAR_EQ(r.y, 800.61965f);
   CHECK_NEAR_EQ(r.z, -38.980828f);
   CHECK_NEAR_EQ(r.w, -295.298807f);
}

TEST_CASE("float * vec4") {
vec4_t a = vec4(-47.124798f ,48.365108f ,-2.354816f ,-17.838881f);
vec4_t b = vec4(16.553662f ,-28.483541f ,-47.156044f ,-47.322734f);
    vec4_t r = a[0] * b;
   CHECK_NEAR_EQ(r.x, -780.087978f);
   CHECK_NEAR_EQ(r.y, 1342.281116f);
   CHECK_NEAR_EQ(r.z, 2222.219048f);
   CHECK_NEAR_EQ(r.w, 2230.074281f);
}

TEST_SUITE_END();
#endif // MATH_VEC_TEST_H
