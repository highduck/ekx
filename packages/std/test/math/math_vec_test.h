
#ifndef MATH_VEC_TEST_H
#define MATH_VEC_TEST_H

#include "math_test_common.h"

TEST_SUITE_BEGIN("vec2");

TEST_CASE("vec2_neg") {
vec2_t a = vec2(24.682311f ,30.360213f);
vec2_t b = vec2(-28.021763f ,-10.416823f);
    vec2_t r = neg_vec2(a);
   CHECK_NEAR_EQ(r.x, -24.682311f);
   CHECK_NEAR_EQ(r.y, -30.360213f);
}

TEST_CASE("vec2_add") {
vec2_t a = vec2(24.682311f ,30.360213f);
vec2_t b = vec2(-28.021763f ,-10.416823f);
    vec2_t r = add_vec2(a, b);
   CHECK_NEAR_EQ(r.x, -3.339452f);
   CHECK_NEAR_EQ(r.y, 19.94339f);
}

TEST_CASE("vec2_sub") {
vec2_t a = vec2(24.682311f ,30.360213f);
vec2_t b = vec2(-28.021763f ,-10.416823f);
    vec2_t r = sub_vec2(a, b);
   CHECK_NEAR_EQ(r.x, 52.704074f);
   CHECK_NEAR_EQ(r.y, 40.777036f);
}

TEST_CASE("vec2_mul") {
vec2_t a = vec2(24.682311f ,30.360213f);
vec2_t b = vec2(-28.021763f ,-10.416823f);
    vec2_t r = mul_vec2(a, b);
   CHECK_NEAR_EQ(r.x, -691.641869f);
   CHECK_NEAR_EQ(r.y, -316.256965f);
}

TEST_CASE("vec2_scale") {
vec2_t a = vec2(24.682311f ,30.360213f);
vec2_t b = vec2(-28.021763f ,-10.416823f);
    vec2_t r = scale_vec2(a, b[0]);
   CHECK_NEAR_EQ(r.x, -691.641869f);
   CHECK_NEAR_EQ(r.y, -850.746693f);
}

TEST_CASE("float * vec2") {
vec2_t a = vec2(24.682311f ,30.360213f);
vec2_t b = vec2(-28.021763f ,-10.416823f);
    vec2_t r = a[0] * b;
   CHECK_NEAR_EQ(r.x, -691.641869f);
   CHECK_NEAR_EQ(r.y, -257.111265f);
}

TEST_SUITE_END();
TEST_SUITE_BEGIN("vec3");

TEST_CASE("vec3_neg") {
vec3_t a = vec3(48.675815f ,21.057057f ,-6.979965f);
vec3_t b = vec3(20.593836f ,29.121432f ,-25.133483f);
    vec3_t r = neg_vec3(a);
   CHECK_NEAR_EQ(r.x, -48.675815f);
   CHECK_NEAR_EQ(r.y, -21.057057f);
   CHECK_NEAR_EQ(r.z, 6.979965f);
}

TEST_CASE("vec3_add") {
vec3_t a = vec3(48.675815f ,21.057057f ,-6.979965f);
vec3_t b = vec3(20.593836f ,29.121432f ,-25.133483f);
    vec3_t r = add_vec3(a, b);
   CHECK_NEAR_EQ(r.x, 69.269651f);
   CHECK_NEAR_EQ(r.y, 50.178489f);
   CHECK_NEAR_EQ(r.z, -32.113448f);
}

TEST_CASE("vec3_sub") {
vec3_t a = vec3(48.675815f ,21.057057f ,-6.979965f);
vec3_t b = vec3(20.593836f ,29.121432f ,-25.133483f);
    vec3_t r = sub_vec3(a, b);
   CHECK_NEAR_EQ(r.x, 28.081979f);
   CHECK_NEAR_EQ(r.y, -8.064375f);
   CHECK_NEAR_EQ(r.z, 18.153518f);
}

TEST_CASE("vec3_mul") {
vec3_t a = vec3(48.675815f ,21.057057f ,-6.979965f);
vec3_t b = vec3(20.593836f ,29.121432f ,-25.133483f);
    vec3_t r = mul_vec3(a, b);
   CHECK_NEAR_EQ(r.x, 1002.421751f);
   CHECK_NEAR_EQ(r.y, 613.211654f);
   CHECK_NEAR_EQ(r.z, 175.430832f);
}

TEST_CASE("vec3_scale") {
vec3_t a = vec3(48.675815f ,21.057057f ,-6.979965f);
vec3_t b = vec3(20.593836f ,29.121432f ,-25.133483f);
    vec3_t r = scale_vec3(a, b[0]);
   CHECK_NEAR_EQ(r.x, 1002.421751f);
   CHECK_NEAR_EQ(r.y, 433.645579f);
   CHECK_NEAR_EQ(r.z, -143.744254f);
}

TEST_CASE("float * vec3") {
vec3_t a = vec3(48.675815f ,21.057057f ,-6.979965f);
vec3_t b = vec3(20.593836f ,29.121432f ,-25.133483f);
    vec3_t r = a[0] * b;
   CHECK_NEAR_EQ(r.x, 1002.421751f);
   CHECK_NEAR_EQ(r.y, 1417.509437f);
   CHECK_NEAR_EQ(r.z, -1223.392769f);
}

TEST_SUITE_END();
TEST_SUITE_BEGIN("vec4");

TEST_CASE("vec4_neg") {
vec4_t a = vec4(7.547078f ,-37.205544f ,-28.72349f ,-44.512186f);
vec4_t b = vec4(-45.594267f ,41.607282f ,-46.617427f ,28.355274f);
    vec4_t r = neg_vec4(a);
   CHECK_NEAR_EQ(r.x, -7.547078f);
   CHECK_NEAR_EQ(r.y, 37.205544f);
   CHECK_NEAR_EQ(r.z, 28.72349f);
   CHECK_NEAR_EQ(r.w, 44.512186f);
}

TEST_CASE("vec4_add") {
vec4_t a = vec4(7.547078f ,-37.205544f ,-28.72349f ,-44.512186f);
vec4_t b = vec4(-45.594267f ,41.607282f ,-46.617427f ,28.355274f);
    vec4_t r = add_vec4(a, b);
   CHECK_NEAR_EQ(r.x, -38.047189f);
   CHECK_NEAR_EQ(r.y, 4.401738f);
   CHECK_NEAR_EQ(r.z, -75.340917f);
   CHECK_NEAR_EQ(r.w, -16.156912f);
}

TEST_CASE("vec4_sub") {
vec4_t a = vec4(7.547078f ,-37.205544f ,-28.72349f ,-44.512186f);
vec4_t b = vec4(-45.594267f ,41.607282f ,-46.617427f ,28.355274f);
    vec4_t r = sub_vec4(a, b);
   CHECK_NEAR_EQ(r.x, 53.141345f);
   CHECK_NEAR_EQ(r.y, -78.812826f);
   CHECK_NEAR_EQ(r.z, 17.893937f);
   CHECK_NEAR_EQ(r.w, -72.86746f);
}

TEST_CASE("vec4_mul") {
vec4_t a = vec4(7.547078f ,-37.205544f ,-28.72349f ,-44.512186f);
vec4_t b = vec4(-45.594267f ,41.607282f ,-46.617427f ,28.355274f);
    vec4_t r = mul_vec4(a, b);
   CHECK_NEAR_EQ(r.x, -344.103489f);
   CHECK_NEAR_EQ(r.y, -1548.021561f);
   CHECK_NEAR_EQ(r.z, 1339.015198f);
   CHECK_NEAR_EQ(r.w, -1262.15523f);
}

TEST_CASE("vec4_scale") {
vec4_t a = vec4(7.547078f ,-37.205544f ,-28.72349f ,-44.512186f);
vec4_t b = vec4(-45.594267f ,41.607282f ,-46.617427f ,28.355274f);
    vec4_t r = scale_vec4(a, b[0]);
   CHECK_NEAR_EQ(r.x, -344.103489f);
   CHECK_NEAR_EQ(r.y, 1696.359507f);
   CHECK_NEAR_EQ(r.z, 1309.626472f);
   CHECK_NEAR_EQ(r.w, 2029.500493f);
}

TEST_CASE("float * vec4") {
vec4_t a = vec4(7.547078f ,-37.205544f ,-28.72349f ,-44.512186f);
vec4_t b = vec4(-45.594267f ,41.607282f ,-46.617427f ,28.355274f);
    vec4_t r = a[0] * b;
   CHECK_NEAR_EQ(r.x, -344.103489f);
   CHECK_NEAR_EQ(r.y, 314.013403f);
   CHECK_NEAR_EQ(r.z, -351.825358f);
   CHECK_NEAR_EQ(r.w, 213.999465f);
}

TEST_SUITE_END();
#endif // MATH_VEC_TEST_H
