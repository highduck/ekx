#include "math_test_common.h"

// Tests are from [HMM](https://github.com/HandmadeMath/Handmade-Math) and adapted

SUITE(mat_math) {

    IT("almost eq") {
        CHECK(almost_eq_f32(0.0000014f, 0.0000019999f, MATH_F32_EPSILON));
        CHECK_FALSE(almost_eq_f32(0.1000014f, 0.2000019999f, MATH_F32_EPSILON));
        CHECK_FALSE(almost_eq_f32(0.0005014f, 0.0004019999f, MATH_F32_EPSILON));
    }

    IT("transformations translate") {

        mat4_t translate = mat4_translate_transform(vec3(1, -3, 6));

        vec3_t original = vec3(1, 2, 3);
        //vec4_t translated = mat4_mul_vec4(translate, vec4(original.x, original.y, original.z, 1));
        vec4_t translated = vec4_mul_mat4(vec4_v(original, 1), translate);

        CHECK_EQ(translated.x, 2.0f);
        CHECK_EQ(translated.y, -1.0f);
        CHECK_EQ(translated.z, 9.0f);
        CHECK_EQ(translated.w, 1.0f);
    }

    IT("transformations rotate") {
        vec3_t original = vec3(1, 1, 1);

        mat4_t rotateX = mat4_rotate(mat4_identity(), to_radians(90), vec3(1, 0, 0));
        vec4_t rotatedX = vec4_mul_mat4(vec4_v(original, 1), rotateX);
        CHECK_NEAR_EQ(rotatedX.x, 1.0f);
        CHECK_NEAR_EQ(rotatedX.y, -1.0f);
        CHECK_NEAR_EQ(rotatedX.z, 1.0f);
        CHECK_NEAR_EQ(rotatedX.w, 1.0f);

        mat4_t rotateY = mat4_rotate(mat4_identity(), to_radians(90), vec3(0, 1, 0));
        vec4_t rotatedY = vec4_mul_mat4(vec4_v(original, 1), rotateY);
        CHECK_NEAR_EQ(rotatedY.x, 1.0f);
        CHECK_NEAR_EQ(rotatedY.y, 1.0f);
        CHECK_NEAR_EQ(rotatedY.z, -1.0f);
        CHECK_NEAR_EQ(rotatedY.w, 1.0f);

        mat4_t rotateZ = mat4_rotate(mat4_identity(), to_radians(90), vec3(0, 0, 1));
        vec4_t rotatedZ = vec4_mul_mat4(vec4_v(original, 1), rotateZ);
        CHECK_NEAR_EQ(rotatedZ.x, -1.0f);
        CHECK_NEAR_EQ(rotatedZ.y, 1.0f);
        CHECK_NEAR_EQ(rotatedZ.z, 1.0f);
        CHECK_NEAR_EQ(rotatedZ.w, 1.0f);
    }

    IT("transformations scale") {
        mat4_t scale = mat4_scale_transform(vec3(2.0f, -3.0f, 0.5f));

        vec3_t original = vec3(1.0f, 2.0f, 3.0f);
        vec4_t scaled = vec4_mul_mat4(vec4_v(original, 1), scale);

        CHECK_NEAR_EQ(scaled.x, 2.0f);
        CHECK_NEAR_EQ(scaled.y, -6.0f);
        CHECK_NEAR_EQ(scaled.z, 1.5f);
        CHECK_NEAR_EQ(scaled.w, 1.0f);
    }

    IT("transformations look at") {
//    const float abs_error = 0.0001f;

        mat4_t result = mat4_look_at_rh(
                vec3(1, 0, 0),
                vec3(0, 2, 1),
                vec3(2, 1, 1)
        );

        CHECK_NEAR_EQ(result.m00, 0.169031f);
        CHECK_NEAR_EQ(result.m01, 0.897085f);
        CHECK_NEAR_EQ(result.m02, 0.408248f);
        CHECK_NEAR_EQ(result.m03, 0.0f);
        CHECK_NEAR_EQ(result.m10, 0.507093f);
        CHECK_NEAR_EQ(result.m11, 0.276026f);
        CHECK_NEAR_EQ(result.m12, -0.816497f);
        CHECK_NEAR_EQ(result.m13, 0.0f);
        CHECK_NEAR_EQ(result.m20, -0.845154f);
        CHECK_NEAR_EQ(result.m21, 0.345033f);
        CHECK_NEAR_EQ(result.m22, -0.408248f);
        CHECK_NEAR_EQ(result.m23, 0.0f);
        CHECK_NEAR_EQ(result.m30, -0.169031f);
        CHECK_NEAR_EQ(result.m31, -0.897085f);
        CHECK_NEAR_EQ(result.m32, -0.408248f);
        CHECK_NEAR_EQ(result.m33, 1.0f);

    }

    IT("Projection: Orthographic") {
        mat4_t projection = mat4_orthographic_rh(-10.0f, 10.0f, -5.0f, 5.0f, 0.0f, -10.0f);

        vec3_t original = vec3(5.0f, 5.0f, -5.0f);
        vec4_t projected = vec4_mul_mat4(vec4_v(original, 1), projection);

        CHECK_NEAR_EQ(projected.x, 0.5f);
        CHECK_NEAR_EQ(projected.y, 1.0f);
//    CHECK_NEAR_EQ(projected.z, -2.0f);
        CHECK_NEAR_EQ(projected.w, 1.0f);
    }

    IT("Projection: Perspective") {
        mat4_t projection = mat4_perspective_lh(to_radians(90), 2.0f, 5.0f, 15.0f);

        {
            vec3_t original = vec3(5.0f, 5.0f, -15.0f);
            vec4_t projected = vec4_mul_mat4(vec4_v(original, 1), projection);
            CHECK_NEAR_EQ(projected.x, 2.5f);
            CHECK_NEAR_EQ(projected.y, 5.0f);
//        CHECK_NEAR_EQ(projected.z, 15.0f);
//        CHECK_NEAR_EQ(projected.w, 15.0f);
        }
        {
            vec3_t original = vec3(5.0f, 5.0f, -5.0f);
            vec4_t projected = vec4_mul_mat4(vec4_v(original, 1), projection);
            CHECK_NEAR_EQ(projected.x, 2.5f);
            CHECK_NEAR_EQ(projected.y, 5.0f);
//        CHECK_NEAR_EQ(projected.z, -5.0f);
//        CHECK_NEAR_EQ(projected.w, 5.0f);
        }
    }
}