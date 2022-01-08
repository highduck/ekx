#ifndef EK_MATH_H
#define EK_MATH_H

// some references:
// - https://github.com/HandmadeMath/Handmade-Math
// - https://github.com/septag/sx/tree/master/include/sx
// - https://github.com/felselva/mathc

#include <stdint.h>
#include <ek/assert.h>
#include <math.h>

#include "math/vec.h"
#include "math/color.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MATH_PI (3.141592653589793238462643383279502884f)
#define MATH_F32_EPSILON (1.0e-6f)
#define MATH_F64_EPSILON (1.0e-8)

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

float to_radians(float degrees);
float to_degrees(float radians);

uint32_t next_power_of_2(uint32_t x);

typedef union quat_t {
    struct {
        float x;
        float y;
        float z;
        float w;
    };
    vec4_t xyzw;
    struct {
        vec3_t xyz;
        float _w;
    };
    float data[4];
} quat_t;

typedef union mat2_t {
    struct {
        float a, b, c, d;
    };
    struct {
        float m00, m01;
        float m10, m11;
    };
    vec4_t v;
    float data[4];
#ifdef __cplusplus

    inline float& operator[](const int index) { return data[index]; }

#endif
} mat2_t;

typedef union mat3x2_t {
    struct {
        float a, b, c, d, tx, ty;
    };
    struct {
        mat2_t rot;
        vec2_t pos;
    };
    float data[6];
    float m[2][3];
#ifdef __cplusplus

    inline float& operator[](const int index) { return data[index]; }

#endif
} mat3x2_t;

typedef union mat3_t {
    struct {
        float m00, m01, m02;
        float m10, m11, m12;
        float m20, m21, m22;
    };
    float data[9];
    vec3_t columns[3];
} mat3_t;

typedef union mat4_t {
    struct {
        float m00, m01, m02, m03;
        float m10, m11, m12, m13;
        float m20, m21, m22, m23;
        float m30, m31, m32, m33;
    };
    float data[16];
    vec4_t columns[4];
} mat4_t;

typedef union rect_t {
    struct {
        float x;
        float y;
        float w;
        float h;
    };
    struct {
        vec2_t position;
        vec2_t size;
    };
    float data[4];
#ifdef __cplusplus

    inline float& operator[](const int index) { return data[index]; }

#endif
} rect_t;

typedef union recti {
    struct {
        int x;
        int y;
        int w;
        int h;
    };
    int data[4];
#ifdef __cplusplus

    inline int& operator[](const int index) { return data[index]; }

#endif
} recti_t;

typedef union rect_i16_t {
    struct {
        int16_t x;
        int16_t y;
        int16_t w;
        int16_t h;
    };
    int16_t data[4];
    uint64_t value;
#ifdef __cplusplus

    inline int16_t& operator[](const int index) { return data[index]; }

#endif
} rect_i16_t;

typedef union brect_t {
    struct {
        float x0;
        float y0;
        float x1;
        float y1;
    };
    struct {
        vec2_t min;
        vec2_t max;
    };
    float data[4];
#ifdef __cplusplus

    inline float& operator[](const int index) { return data[index]; }

#endif
} brect_t;

typedef union circle_t {
    struct {
        vec2_t center;
        float radius;
    };
    struct {
        float x;
        float y;
        float r;
    };
    float data[3];
#ifdef __cplusplus

    inline float& operator[](const int index) { return data[index]; }

#endif
} circle_t;

typedef union aabb_t {
    struct {
        float x0;
        float y0;
        float z0;
        float x1;
        float y1;
        float z1;
    };
    struct {
        vec3_t min;
        vec3_t max;
    };
    float data[6];
#ifdef __cplusplus

    inline float& operator[](const int index) { return data[index]; }

#endif
} aabb_t;

vec2_t vec2_transform(vec2_t point, mat3x2_t matrix);

bool vec2_transform_inverse(vec2_t point, mat3x2_t matrix, vec2_t* out);
quat_t quat_normalize(quat_t a);

float f32_lerp(float a, float b, float t);

bool almost_eq_f32(float a, float b, float eps);

inline static float clamp(float x, float min, float max) {
    return x > max ? max : (x < min ? min : x);
}

inline static float saturate(float x) {
    return clamp(x, 0, 1);
}

/** mat2 **/
mat2_t mat2_scale_skew(vec2_t scale, vec2_t skew);
void mat2_scale(mat2_t* mat, vec2_t scale);
void mat2_rotate(mat2_t* mat, float radians);
mat2_t mat2_identity(void);
vec2_t mat2_get_scale(mat2_t m);
vec2_t mat2_get_skew(mat2_t m);
float mat2_get_rotation(mat2_t m);
float mat2_det(mat2_t m);
mat2_t mat2_affine_inverse(mat2_t m);

/** mat3x2 **/
void mat3x2_translate(mat3x2_t* mat, vec2_t v);
void mat3x2_scale(mat3x2_t* mat, vec2_t scale);
void mat3x2_rotate(mat3x2_t* mat, float radians);
void mat3x2_transform_pivot(mat3x2_t* mat, vec2_t pos, float rot, vec2_t scale, vec2_t pivot);
mat3x2_t mat3x2_mul(mat3x2_t left, mat3x2_t right);
mat3x2_t mat3x2_identity(void);
vec2_t mat3x2_get_scale(mat3x2_t m);
vec2_t mat3x2_get_skew(mat3x2_t m);
float mat3x2_get_rotation(mat3x2_t m);
float mat3x2_det(mat3x2_t m);
bool mat3x2_inverse(mat3x2_t* m);

/** mat4x4 **/
mat4_t mat4_identity(void);
mat4_t mat4_d(float d);
mat4_t mat4_mat3(mat3_t m);
mat4_t mat4_2d_transform(vec2_t pos, vec2_t scale, vec2_t skew);
mat4_t mat4_perspective_rh(float fov_y, float aspect, float z_near, float z_far);
mat4_t mat4_perspective_lh(float fov_y, float aspect, float z_near, float z_far);
mat4_t mat4_orthographic_lh(float left, float right, float bottom, float top, float z_near, float z_far);
mat4_t mat4_orthographic_rh(float left, float right, float bottom, float top, float z_near, float z_far);
mat4_t mat4_orthographic_2d(float x, float y, float w, float h, float z_near, float z_far);
mat4_t mat4_look_at_rh(vec3_t eye, vec3_t center, vec3_t up);
mat4_t mat4_look_at_lh(vec3_t eye, vec3_t center, vec3_t up);
mat4_t mat4_mul(mat4_t l, mat4_t r);
mat4_t mat4_mul_mat3x2(mat4_t l, mat3x2_t r);
mat4_t mat4_inverse(mat4_t m);

mat3_t mat4_get_mat3(mat4_t* m);
mat3_t mat3_inverse(mat3_t m);
mat3_t mat3_transpose(mat3_t m);
mat4_t mat4_transpose(mat4_t m);

vec3_t mat4_get_position(const mat4_t* m);

mat4_t mat4_rotate(mat4_t m, float angle, vec3_t v);
mat4_t mat4_rotation_transform_xzy(vec3_t v);
mat4_t mat4_rotation_transform_quat(quat_t q);
mat4_t mat4_translate_transform(vec3_t translation);
mat4_t mat4_scale_transform(vec3_t scale);

// Post-multiply
vec4_t mat4_mul_vec4(mat4_t m, vec4_t v);
vec3_t mat4_mul_vec3(mat4_t m, vec3_t v);

// Pre-multiply
vec4_t vec4_mul_mat4(vec4_t v, mat4_t m);
vec3_t vec3_mul_mat4(vec3_t v, mat4_t m);

/** quaternion **/

quat_t quat_euler_angles(vec3_t angles);
vec3_t quat_to_euler_angles(quat_t q);
quat_t quat_mat4(mat4_t m);
quat_t quat_look_at_rh(vec3_t direction, vec3_t up);
float quat_roll(quat_t q);
float quat_pitch(quat_t q);
float quat_yaw(quat_t q);

/** u8 functions **/

uint8_t unorm8_f32(float f);
uint8_t unorm8_f32_clamped(float f);
uint8_t u8_norm_mul(uint8_t a, uint8_t b);
uint8_t u8_add_sat(uint8_t a, uint8_t b);

rect_t rect(float x, float y, float w, float h);
rect_t recti_to_rect(recti_t irc);
rect_t rect_size(vec2_t size);
rect_t rect_minmax(vec2_t min, vec2_t max);
rect_t rect_01(void);
rect_t rect_wh(float w, float h);
vec2_t rect_rb(rect_t rc);
vec2_t rect_center(rect_t rc);
rect_t rect_clamp_bounds(rect_t a, rect_t b);
recti_t recti_clamp_bounds(recti_t a, recti_t b);
//#define rect_clamp_bounds(A, B) {{MAX((A).x, (B).x),MAX((A).y, (B).y),MIN(RECT_R(A), RECT_R(B)) - MAX((A).x, (B).x),MIN(RECT_B(A), RECT_B(B)) - MAX((A).y, (B).y)}}
//#define rectangle_clamp_bounds(A, B) {{MAX((A).x, (B).x),MAX((A).y, (B).y),MIN((A).r, (B).r),MIN((A).b, (B).b)}}

rect_t rect_combine(rect_t a, rect_t b);
recti_t recti_combine(recti_t a, recti_t b);
bool rect_overlaps(rect_t a, rect_t b);
bool recti_overlaps(recti_t a, recti_t b);
rect_t rect_scale(rect_t a, vec2_t s);
rect_t rect_scale_f(rect_t a, float s);
rect_t rect_translate(rect_t a, vec2_t t);

/**
 * Transforms rectangle points to calculate it's axis-aligned bounds
 * @param rect
 * @param matrix
 * @return axis-aligned bounding rect of transformed rectangle shape
 */
rect_t rect_transform(rect_t a, mat3x2_t matrix);

bool rect_contains(rect_t rc, vec2_t point);
bool rect_is_empty(rect_t a);
rect_t rect_expand(rect_t a, float w);

#define RECT_R(rc) ((rc).x + (rc).w)
#define RECT_B(rc) ((rc).y + (rc).h)
#define RECT_CENTER_X(rc) ((rc).x + (rc).w * 0.5f)
#define RECT_CENTER_Y(rc) ((rc).y + (rc).h * 0.5f)
//#define rect_overlaps(a, b) ((a).x <= RECT_R(b) && (b).x <= RECT_R(a) && (a).y <= RECT_B(b) && (b).y <= RECT_B(a))

circle_t circle(float x, float y, float r);

brect_t brect_from_rect(rect_t rc);
brect_t brect_inf(void);
brect_t brect_extend_circle(brect_t bb, circle_t circle);
brect_t brect_extend_rect(brect_t bb, rect_t rc);
brect_t brect_extend_transformed_rect(brect_t bb, rect_t rc, mat3x2_t matrix);
brect_t brect_extend_point(brect_t bb, vec2_t point);
rect_t brect_get_rect(brect_t bb);

bool brect_is_empty(brect_t bb);

#ifdef __cplusplus
}

#include "math/rect.hpp"

#endif

#endif // EK_MATH_H
