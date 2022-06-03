
#ifndef MATH_VEC_TEST_H
#define MATH_VEC_TEST_H

#include "math_test_common.h"

SUITE(vec_math) {
	DESCRIBE(vec2) {

		IT("has neg") {
			vec2_t a = vec2(-24.276745f ,-16.452579f);
			vec2_t b = vec2(20.711396f ,-14.575699f);
			vec2_t r = neg_vec2(a);
			CHECK_NEAR_EQ(r.x, 24.276745f);
			CHECK_NEAR_EQ(r.y, 16.452579f);
		}

		IT("has add") {
			vec2_t a = vec2(-24.276745f ,-16.452579f);
			vec2_t b = vec2(20.711396f ,-14.575699f);
			vec2_t r = add_vec2(a, b);
			CHECK_NEAR_EQ(r.x, -3.565349f);
			CHECK_NEAR_EQ(r.y, -31.028278f);
		}

		IT("has sub") {
			vec2_t a = vec2(-24.276745f ,-16.452579f);
			vec2_t b = vec2(20.711396f ,-14.575699f);
			vec2_t r = sub_vec2(a, b);
			CHECK_NEAR_EQ(r.x, -44.988141f);
			CHECK_NEAR_EQ(r.y, -1.87688f);
		}

		IT("has mul") {
			vec2_t a = vec2(-24.276745f ,-16.452579f);
			vec2_t b = vec2(20.711396f ,-14.575699f);
			vec2_t r = mul_vec2(a, b);
			CHECK_NEAR_EQ(r.x, -502.805279f);
			CHECK_NEAR_EQ(r.y, 239.807839f);
		}

		IT("has scale") {
			vec2_t a = vec2(-24.276745f ,-16.452579f);
			vec2_t b = vec2(20.711396f ,-14.575699f);
			vec2_t r = scale_vec2(a, b.x);
			CHECK_NEAR_EQ(r.x, -502.805279f);
			CHECK_NEAR_EQ(r.y, -340.755879f);
		}

	}
    DESCRIBE(vec3) {

		IT("has neg") {
			vec3_t a = vec3(-27.645848f ,-8.273022f ,-7.235213f);
			vec3_t b = vec3(27.67516f ,-18.385148f ,-39.476274f);
			vec3_t r = neg_vec3(a);
			CHECK_NEAR_EQ(r.x, 27.645848f);
			CHECK_NEAR_EQ(r.y, 8.273022f);
			CHECK_NEAR_EQ(r.z, 7.235213f);
		}

		IT("has add") {
			vec3_t a = vec3(-27.645848f ,-8.273022f ,-7.235213f);
			vec3_t b = vec3(27.67516f ,-18.385148f ,-39.476274f);
			vec3_t r = add_vec3(a, b);
			CHECK_NEAR_EQ(r.x, 0.029312f);
			CHECK_NEAR_EQ(r.y, -26.65817f);
			CHECK_NEAR_EQ(r.z, -46.711487f);
		}

		IT("has sub") {
			vec3_t a = vec3(-27.645848f ,-8.273022f ,-7.235213f);
			vec3_t b = vec3(27.67516f ,-18.385148f ,-39.476274f);
			vec3_t r = sub_vec3(a, b);
			CHECK_NEAR_EQ(r.x, -55.321008f);
			CHECK_NEAR_EQ(r.y, 10.112126f);
			CHECK_NEAR_EQ(r.z, 32.241061f);
		}

		IT("has mul") {
			vec3_t a = vec3(-27.645848f ,-8.273022f ,-7.235213f);
			vec3_t b = vec3(27.67516f ,-18.385148f ,-39.476274f);
			vec3_t r = mul_vec3(a, b);
			CHECK_NEAR_EQ(r.x, -765.103267f);
			CHECK_NEAR_EQ(r.y, 152.100734f);
			CHECK_NEAR_EQ(r.z, 285.619251f);
		}

		IT("has scale") {
			vec3_t a = vec3(-27.645848f ,-8.273022f ,-7.235213f);
			vec3_t b = vec3(27.67516f ,-18.385148f ,-39.476274f);
			vec3_t r = scale_vec3(a, b.x);
			CHECK_NEAR_EQ(r.x, -765.103267f);
			CHECK_NEAR_EQ(r.y, -228.957208f);
			CHECK_NEAR_EQ(r.z, -200.235677f);
		}

	}
    DESCRIBE(vec4) {

		IT("has neg") {
			vec4_t a = vec4(3.818989f ,-11.800797f ,-8.425165f ,-5.453932f);
			vec4_t b = vec4(1.445342f ,42.543233f ,-39.707165f ,-45.973427f);
			vec4_t r = neg_vec4(a);
			CHECK_NEAR_EQ(r.x, -3.818989f);
			CHECK_NEAR_EQ(r.y, 11.800797f);
			CHECK_NEAR_EQ(r.z, 8.425165f);
			CHECK_NEAR_EQ(r.w, 5.453932f);
		}

		IT("has add") {
			vec4_t a = vec4(3.818989f ,-11.800797f ,-8.425165f ,-5.453932f);
			vec4_t b = vec4(1.445342f ,42.543233f ,-39.707165f ,-45.973427f);
			vec4_t r = add_vec4(a, b);
			CHECK_NEAR_EQ(r.x, 5.264331f);
			CHECK_NEAR_EQ(r.y, 30.742436f);
			CHECK_NEAR_EQ(r.z, -48.13233f);
			CHECK_NEAR_EQ(r.w, -51.427359f);
		}

		IT("has sub") {
			vec4_t a = vec4(3.818989f ,-11.800797f ,-8.425165f ,-5.453932f);
			vec4_t b = vec4(1.445342f ,42.543233f ,-39.707165f ,-45.973427f);
			vec4_t r = sub_vec4(a, b);
			CHECK_NEAR_EQ(r.x, 2.373647f);
			CHECK_NEAR_EQ(r.y, -54.34403f);
			CHECK_NEAR_EQ(r.z, 31.282f);
			CHECK_NEAR_EQ(r.w, 40.519495f);
		}

		IT("has mul") {
			vec4_t a = vec4(3.818989f ,-11.800797f ,-8.425165f ,-5.453932f);
			vec4_t b = vec4(1.445342f ,42.543233f ,-39.707165f ,-45.973427f);
			vec4_t r = mul_vec4(a, b);
			CHECK_NEAR_EQ(r.x, 5.519745f);
			CHECK_NEAR_EQ(r.y, -502.044056f);
			CHECK_NEAR_EQ(r.z, 334.539417f);
			CHECK_NEAR_EQ(r.w, 250.735945f);
		}

		IT("has scale") {
			vec4_t a = vec4(3.818989f ,-11.800797f ,-8.425165f ,-5.453932f);
			vec4_t b = vec4(1.445342f ,42.543233f ,-39.707165f ,-45.973427f);
			vec4_t r = scale_vec4(a, b.x);
			CHECK_NEAR_EQ(r.x, 5.519745f);
			CHECK_NEAR_EQ(r.y, -17.056188f);
			CHECK_NEAR_EQ(r.z, -12.177245f);
			CHECK_NEAR_EQ(r.w, -7.882797f);
		}

	}
}
#endif // MATH_VEC_TEST_H
