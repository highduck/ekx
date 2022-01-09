
#ifndef MATH_VEC_TEST_H
#define MATH_VEC_TEST_H

#include "math_test_common.h"

TEST_SUITE_BEGIN("vec2");

TEST_CASE("vec2_neg") {
vec2_t a = vec2(19.2f ,-40.5f);
vec2_t b = vec2(11.6f ,-13.0f);
    vec2_t r = neg_vec2(a);
   CHECK_NEAR_EQ(r.x, -19.2f);
   CHECK_NEAR_EQ(r.y, 40.5f);
}

TEST_CASE("vec2_add") {
vec2_t a = vec2(19.2f ,-40.5f);
vec2_t b = vec2(11.6f ,-13.0f);
    vec2_t r = add_vec2(a, b);
   CHECK_NEAR_EQ(r.x, 30.799999999999997f);
   CHECK_NEAR_EQ(r.y, -53.5f);
}

TEST_CASE("vec2_sub") {
vec2_t a = vec2(19.2f ,-40.5f);
vec2_t b = vec2(11.6f ,-13.0f);
    vec2_t r = sub_vec2(a, b);
   CHECK_NEAR_EQ(r.x, 7.6f);
   CHECK_NEAR_EQ(r.y, -27.5f);
}

TEST_CASE("vec2_mul") {
vec2_t a = vec2(19.2f ,-40.5f);
vec2_t b = vec2(11.6f ,-13.0f);
    vec2_t r = mul_vec2(a, b);
   CHECK_NEAR_EQ(r.x, 222.72f);
   CHECK_NEAR_EQ(r.y, 526.5f);
}

TEST_CASE("vec2_scale") {
vec2_t a = vec2(19.2f ,-40.5f);
vec2_t b = vec2(11.6f ,-13.0f);
    vec2_t r = scale_vec2(a, b[0]);
   CHECK_NEAR_EQ(r.x, 222.72f);
   CHECK_NEAR_EQ(r.y, -469.8f);
}

TEST_CASE("float * vec2") {
vec2_t a = vec2(19.2f ,-40.5f);
vec2_t b = vec2(11.6f ,-13.0f);
    vec2_t r = a[0] * b;
   CHECK_NEAR_EQ(r.x, 222.72f);
   CHECK_NEAR_EQ(r.y, -249.6f);
}

TEST_SUITE_END();
TEST_SUITE_BEGIN("vec3");

TEST_CASE("vec3_neg") {
vec3_t a = vec3(43.3f ,-48.8f ,-47.1f);
vec3_t b = vec3(-29.3f ,10.8f ,0.9f);
    vec3_t r = neg_vec3(a);
   CHECK_NEAR_EQ(r.x, -43.3f);
   CHECK_NEAR_EQ(r.y, 48.8f);
   CHECK_NEAR_EQ(r.z, 47.1f);
}

TEST_CASE("vec3_add") {
vec3_t a = vec3(43.3f ,-48.8f ,-47.1f);
vec3_t b = vec3(-29.3f ,10.8f ,0.9f);
    vec3_t r = add_vec3(a, b);
   CHECK_NEAR_EQ(r.x, 13.999999999999996f);
   CHECK_NEAR_EQ(r.y, -38.0f);
   CHECK_NEAR_EQ(r.z, -46.2f);
}

TEST_CASE("vec3_sub") {
vec3_t a = vec3(43.3f ,-48.8f ,-47.1f);
vec3_t b = vec3(-29.3f ,10.8f ,0.9f);
    vec3_t r = sub_vec3(a, b);
   CHECK_NEAR_EQ(r.x, 72.6f);
   CHECK_NEAR_EQ(r.y, -59.599999999999994f);
   CHECK_NEAR_EQ(r.z, -48.0f);
}

TEST_CASE("vec3_mul") {
vec3_t a = vec3(43.3f ,-48.8f ,-47.1f);
vec3_t b = vec3(-29.3f ,10.8f ,0.9f);
    vec3_t r = mul_vec3(a, b);
   CHECK_NEAR_EQ(r.x, -1268.69f);
   CHECK_NEAR_EQ(r.y, -527.04f);
   CHECK_NEAR_EQ(r.z, -42.39f);
}

TEST_CASE("vec3_scale") {
vec3_t a = vec3(43.3f ,-48.8f ,-47.1f);
vec3_t b = vec3(-29.3f ,10.8f ,0.9f);
    vec3_t r = scale_vec3(a, b[0]);
   CHECK_NEAR_EQ(r.x, -1268.69f);
   CHECK_NEAR_EQ(r.y, 1429.84f);
   CHECK_NEAR_EQ(r.z, 1380.03f);
}

TEST_CASE("float * vec3") {
vec3_t a = vec3(43.3f ,-48.8f ,-47.1f);
vec3_t b = vec3(-29.3f ,10.8f ,0.9f);
    vec3_t r = a[0] * b;
   CHECK_NEAR_EQ(r.x, -1268.69f);
   CHECK_NEAR_EQ(r.y, 467.64f);
   CHECK_NEAR_EQ(r.z, 38.97f);
}

TEST_SUITE_END();
TEST_SUITE_BEGIN("vec4");

TEST_CASE("vec4_neg") {
vec4_t a = vec4(19.5f ,47.2f ,5.5f ,13.7f);
vec4_t b = vec4(32.6f ,-24.9f ,-21.3f ,-49.0f);
    vec4_t r = neg_vec4(a);
   CHECK_NEAR_EQ(r.x, -19.5f);
   CHECK_NEAR_EQ(r.y, -47.2f);
   CHECK_NEAR_EQ(r.z, -5.5f);
   CHECK_NEAR_EQ(r.w, -13.7f);
}

TEST_CASE("vec4_add") {
vec4_t a = vec4(19.5f ,47.2f ,5.5f ,13.7f);
vec4_t b = vec4(32.6f ,-24.9f ,-21.3f ,-49.0f);
    vec4_t r = add_vec4(a, b);
   CHECK_NEAR_EQ(r.x, 52.1f);
   CHECK_NEAR_EQ(r.y, 22.300000000000004f);
   CHECK_NEAR_EQ(r.z, -15.8f);
   CHECK_NEAR_EQ(r.w, -35.3f);
}

TEST_CASE("vec4_sub") {
vec4_t a = vec4(19.5f ,47.2f ,5.5f ,13.7f);
vec4_t b = vec4(32.6f ,-24.9f ,-21.3f ,-49.0f);
    vec4_t r = sub_vec4(a, b);
   CHECK_NEAR_EQ(r.x, -13.100000000000001f);
   CHECK_NEAR_EQ(r.y, 72.1f);
   CHECK_NEAR_EQ(r.z, 26.8f);
   CHECK_NEAR_EQ(r.w, 62.7f);
}

TEST_CASE("vec4_mul") {
vec4_t a = vec4(19.5f ,47.2f ,5.5f ,13.7f);
vec4_t b = vec4(32.6f ,-24.9f ,-21.3f ,-49.0f);
    vec4_t r = mul_vec4(a, b);
   CHECK_NEAR_EQ(r.x, 635.7f);
   CHECK_NEAR_EQ(r.y, -1175.28f);
   CHECK_NEAR_EQ(r.z, -117.15f);
   CHECK_NEAR_EQ(r.w, -671.3f);
}

TEST_CASE("vec4_scale") {
vec4_t a = vec4(19.5f ,47.2f ,5.5f ,13.7f);
vec4_t b = vec4(32.6f ,-24.9f ,-21.3f ,-49.0f);
    vec4_t r = scale_vec4(a, b[0]);
   CHECK_NEAR_EQ(r.x, 635.7f);
   CHECK_NEAR_EQ(r.y, 1538.7200000000003f);
   CHECK_NEAR_EQ(r.z, 179.3f);
   CHECK_NEAR_EQ(r.w, 446.62f);
}

TEST_CASE("float * vec4") {
vec4_t a = vec4(19.5f ,47.2f ,5.5f ,13.7f);
vec4_t b = vec4(32.6f ,-24.9f ,-21.3f ,-49.0f);
    vec4_t r = a[0] * b;
   CHECK_NEAR_EQ(r.x, 635.7f);
   CHECK_NEAR_EQ(r.y, -485.54999999999995f);
   CHECK_NEAR_EQ(r.z, -415.35f);
   CHECK_NEAR_EQ(r.w, -955.5f);
}

TEST_SUITE_END();
#endif // MATH_VEC_TEST_H
