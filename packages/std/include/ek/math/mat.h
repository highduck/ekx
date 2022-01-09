#ifndef EK_MATH_MAT_H
#define EK_MATH_MAT_H

#ifdef __cplusplus
extern "C" {
#endif


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

vec2_t vec2_transform(vec2_t point, mat3x2_t matrix);
bool vec2_transform_inverse(vec2_t point, mat3x2_t matrix, vec2_t* out);

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
mat4_t mat4_translate_transform(vec3_t translation);
mat4_t mat4_scale_transform(vec3_t scale);

// Post-multiply
vec4_t mat4_mul_vec4(mat4_t m, vec4_t v);
vec3_t mat4_mul_vec3(mat4_t m, vec3_t v);

// Pre-multiply
vec4_t vec4_mul_mat4(vec4_t v, mat4_t m);
vec3_t vec3_mul_mat4(vec3_t v, mat4_t m);

#ifdef __cplusplus
}
#endif

#endif // EK_MATH_MAT_H
