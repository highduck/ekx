
#ifndef MATH_VEC_TEST_H
#define MATH_VEC_TEST_H

#include "math_test_common.h"

TEST_SUITE_BEGIN("vec2");

TEST_CASE("vec2_neg") {
vec2_t a = vec2(-2.1f ,10.9f);
vec2_t b = vec2(33.3f ,40.9f);
    vec2_t r = vec2_neg(a);
   CHECK_NEAR_EQ(r.x, 2.1f);
   CHECK_NEAR_EQ(r.y, -10.9f);
}

TEST_CASE("vec2_add") {
vec2_t a = vec2(-2.1f ,10.9f);
vec2_t b = vec2(33.3f ,40.9f);
    vec2_t r = vec2_add(a, b);
   CHECK_NEAR_EQ(r.x, 31.199999999999996f);
   CHECK_NEAR_EQ(r.y, 51.8f);
}

TEST_CASE("vec2_sub") {
vec2_t a = vec2(-2.1f ,10.9f);
vec2_t b = vec2(33.3f ,40.9f);
    vec2_t r = vec2_sub(a, b);
   CHECK_NEAR_EQ(r.x, -35.4f);
   CHECK_NEAR_EQ(r.y, -30.0f);
}

TEST_CASE("vec2_mul") {
vec2_t a = vec2(-2.1f ,10.9f);
vec2_t b = vec2(33.3f ,40.9f);
    vec2_t r = vec2_mul(a, b);
   CHECK_NEAR_EQ(r.x, -69.92999999999999f);
   CHECK_NEAR_EQ(r.y, 445.81f);
}

TEST_CASE("vec2_scale") {
vec2_t a = vec2(-2.1f ,10.9f);
vec2_t b = vec2(33.3f ,40.9f);
    vec2_t r = vec2_scale(a, b[0]);
   CHECK_NEAR_EQ(r.x, -69.92999999999999f);
   CHECK_NEAR_EQ(r.y, 362.96999999999997f);
}

TEST_CASE("float * vec2") {
vec2_t a = vec2(-2.1f ,10.9f);
vec2_t b = vec2(33.3f ,40.9f);
    vec2_t r = a[0] * b;
   CHECK_NEAR_EQ(r.x, -69.92999999999999f);
   CHECK_NEAR_EQ(r.y, -85.89f);
}

TEST_SUITE_END();
TEST_SUITE_BEGIN("vec3");

TEST_CASE("vec3_neg") {
vec3_t a = vec3(-8.4f ,47.7f ,-29.3f);
vec3_t b = vec3(31.4f ,-6.8f ,2.7f);
    vec3_t r = vec3_neg(a);
   CHECK_NEAR_EQ(r.x, 8.4f);
   CHECK_NEAR_EQ(r.y, -47.7f);
   CHECK_NEAR_EQ(r.z, 29.3f);
}

TEST_CASE("vec3_add") {
vec3_t a = vec3(-8.4f ,47.7f ,-29.3f);
vec3_t b = vec3(31.4f ,-6.8f ,2.7f);
    vec3_t r = vec3_add(a, b);
   CHECK_NEAR_EQ(r.x, 23.0f);
   CHECK_NEAR_EQ(r.y, 40.900000000000006f);
   CHECK_NEAR_EQ(r.z, -26.6f);
}

TEST_CASE("vec3_sub") {
vec3_t a = vec3(-8.4f ,47.7f ,-29.3f);
vec3_t b = vec3(31.4f ,-6.8f ,2.7f);
    vec3_t r = vec3_sub(a, b);
   CHECK_NEAR_EQ(r.x, -39.8f);
   CHECK_NEAR_EQ(r.y, 54.5f);
   CHECK_NEAR_EQ(r.z, -32.0f);
}

TEST_CASE("vec3_mul") {
vec3_t a = vec3(-8.4f ,47.7f ,-29.3f);
vec3_t b = vec3(31.4f ,-6.8f ,2.7f);
    vec3_t r = vec3_mul(a, b);
   CHECK_NEAR_EQ(r.x, -263.76f);
   CHECK_NEAR_EQ(r.y, -324.36f);
   CHECK_NEAR_EQ(r.z, -79.11000000000001f);
}

TEST_CASE("vec3_scale") {
vec3_t a = vec3(-8.4f ,47.7f ,-29.3f);
vec3_t b = vec3(31.4f ,-6.8f ,2.7f);
    vec3_t r = vec3_scale(a, b[0]);
   CHECK_NEAR_EQ(r.x, -263.76f);
   CHECK_NEAR_EQ(r.y, 1497.78f);
   CHECK_NEAR_EQ(r.z, -920.02f);
}

TEST_CASE("float * vec3") {
vec3_t a = vec3(-8.4f ,47.7f ,-29.3f);
vec3_t b = vec3(31.4f ,-6.8f ,2.7f);
    vec3_t r = a[0] * b;
   CHECK_NEAR_EQ(r.x, -263.76f);
   CHECK_NEAR_EQ(r.y, 57.12f);
   CHECK_NEAR_EQ(r.z, -22.680000000000003f);
}

TEST_SUITE_END();
TEST_SUITE_BEGIN("vec4");

TEST_CASE("vec4_neg") {
vec4_t a = vec4(-2.5f ,26.4f ,-8.5f ,-43.1f);
vec4_t b = vec4(38.4f ,30.6f ,31.7f ,25.3f);
    vec4_t r = vec4_neg(a);
   CHECK_NEAR_EQ(r.x, 2.5f);
   CHECK_NEAR_EQ(r.y, -26.4f);
   CHECK_NEAR_EQ(r.z, 8.5f);
   CHECK_NEAR_EQ(r.w, 43.1f);
}

TEST_CASE("vec4_add") {
vec4_t a = vec4(-2.5f ,26.4f ,-8.5f ,-43.1f);
vec4_t b = vec4(38.4f ,30.6f ,31.7f ,25.3f);
    vec4_t r = vec4_add(a, b);
   CHECK_NEAR_EQ(r.x, 35.9f);
   CHECK_NEAR_EQ(r.y, 57.0f);
   CHECK_NEAR_EQ(r.z, 23.2f);
   CHECK_NEAR_EQ(r.w, -17.8f);
}

TEST_CASE("vec4_sub") {
vec4_t a = vec4(-2.5f ,26.4f ,-8.5f ,-43.1f);
vec4_t b = vec4(38.4f ,30.6f ,31.7f ,25.3f);
    vec4_t r = vec4_sub(a, b);
   CHECK_NEAR_EQ(r.x, -40.9f);
   CHECK_NEAR_EQ(r.y, -4.200000000000003f);
   CHECK_NEAR_EQ(r.z, -40.2f);
   CHECK_NEAR_EQ(r.w, -68.4f);
}

TEST_CASE("vec4_mul") {
vec4_t a = vec4(-2.5f ,26.4f ,-8.5f ,-43.1f);
vec4_t b = vec4(38.4f ,30.6f ,31.7f ,25.3f);
    vec4_t r = vec4_mul(a, b);
   CHECK_NEAR_EQ(r.x, -96.0f);
   CHECK_NEAR_EQ(r.y, 807.84f);
   CHECK_NEAR_EQ(r.z, -269.45f);
   CHECK_NEAR_EQ(r.w, -1090.43f);
}

TEST_CASE("vec4_scale") {
vec4_t a = vec4(-2.5f ,26.4f ,-8.5f ,-43.1f);
vec4_t b = vec4(38.4f ,30.6f ,31.7f ,25.3f);
    vec4_t r = vec4_scale(a, b[0]);
   CHECK_NEAR_EQ(r.x, -96.0f);
   CHECK_NEAR_EQ(r.y, 1013.7599999999999f);
   CHECK_NEAR_EQ(r.z, -326.4f);
   CHECK_NEAR_EQ(r.w, -1655.04f);
}

TEST_CASE("float * vec4") {
vec4_t a = vec4(-2.5f ,26.4f ,-8.5f ,-43.1f);
vec4_t b = vec4(38.4f ,30.6f ,31.7f ,25.3f);
    vec4_t r = a[0] * b;
   CHECK_NEAR_EQ(r.x, -96.0f);
   CHECK_NEAR_EQ(r.y, -76.5f);
   CHECK_NEAR_EQ(r.z, -79.25f);
   CHECK_NEAR_EQ(r.w, -63.25f);
}

TEST_SUITE_END();
#endif // MATH_VEC_TEST_H
