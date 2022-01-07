#include <ek/math.h>
#include <ek/assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/** mat2 **/

mat2_t mat2_scale_skew(vec2_t scale, vec2_t skew) {
    mat2_t r;
    r.a = cosf(skew.y) * scale.x;
    r.b = sinf(skew.y) * scale.x;
    r.c = -sinf(skew.x) * scale.y;
    r.d = cosf(skew.x) * scale.y;
    return r;
}

void mat2_scale(mat2_t* mat, const vec2_t scale) {
    mat->a *= scale.x;
    mat->b *= scale.x;
    mat->c *= scale.y;
    mat->d *= scale.y;
}

void mat2_rotate(mat2_t* mat, float radians) {
    const float sn = sinf(radians);
    const float cs = cosf(radians);
    const float pa = mat->a;
    const float pb = mat->b;
    const float pc = mat->c;
    const float pd = mat->d;
    mat->a = pa * cs + pc * sn;
    mat->c = -pa * sn + pc * cs;
    mat->b = pd * sn + pb * cs;
    mat->d = pd * cs - pb * sn;
}

mat2_t mat2_identity(void) {
    return (mat2_t) {{1, 0, 0, 1}};
}

vec2_t mat2_get_scale(mat2_t m) {
    vec2_t r;
    r.x = sqrtf(m.a * m.a + m.b * m.b);
    r.y = sqrtf(m.c * m.c + m.d * m.d);
    // if (a < 0.0f) s.x = -s.x;
    // if (d < 0.0f) s.y = -s.y;
    return r;
}

vec2_t mat2_get_skew(mat2_t m) {
    vec2_t r;
    r.x = atan2f(-m.c, m.d);
    r.y = atan2f(m.b, m.a);
    return r;
}

float mat2_get_rotation(mat2_t m) {
    const vec2_t skew = mat2_get_skew(m);
    return skew.x == skew.y ? skew.y : 0.0f;
}

float mat2_det(mat2_t m) {
    return m.a * m.d - m.c * m.b;
}


/** mat3x2 **/
void mat3x2_translate(mat3x2_t* mat, vec2_t v) {
    mat->tx += mat->a * v.x + mat->c * v.y;
    mat->ty += mat->d * v.y + mat->b * v.x;
}

void mat3x2_scale(mat3x2_t* mat, vec2_t scale) {
    mat2_scale(&mat->rot, scale);
}

void mat3x2_rotate(mat3x2_t* mat, float radians) {
    mat2_rotate(&mat->rot, radians);
}

void mat3x2_transform_pivot(mat3x2_t* mat, vec2_t pos, float rot, vec2_t scale, vec2_t pivot) {
    mat3x2_translate(mat, vec2_add(pos, pivot));
    mat3x2_scale(mat, scale);
    mat3x2_rotate(mat, rot);
    mat3x2_translate(mat, vec2_neg(pivot));
}

// NOTES: to do `concat` style method - just swap right and left matrices,
// it will act like `this` is right, `m` is left (pre-multiplication)
mat3x2_t mat3x2_mul(const mat3x2_t l, const mat3x2_t r) {
    mat3x2_t m;
    m.a = l.a * r.a + l.c * r.b;
    m.b = l.b * r.a + l.d * r.b;
    m.c = l.a * r.c + l.c * r.d;
    m.d = l.b * r.c + l.d * r.d;
    m.tx = l.a * r.tx + l.c * r.ty + l.tx;
    m.ty = l.b * r.tx + l.d * r.ty + l.ty;
    return m;
}

mat3x2_t mat3x2_identity(void) {
    mat3x2_t m;
    m.data[0] = 1.0f;
    m.data[1] = 0.0f;
    m.data[2] = 0.0f;
    m.data[3] = 1.0f;
    m.data[4] = 0.0f;
    m.data[5] = 0.0f;
    return m;
}

vec2_t mat3x2_get_scale(const mat3x2_t m) {
    return mat2_get_scale(m.rot);
}

vec2_t mat3x2_get_skew(const mat3x2_t m) {
    return mat2_get_skew(m.rot);
}

float mat3x2_get_rotation(const mat3x2_t m) {
    return mat2_get_rotation(m.rot);
}

float mat3x2_det(const mat3x2_t m) {
    return mat2_det(m.rot);
}

bool mat3x2_inverse(mat3x2_t* inout_matrix) {
    const mat3x2_t m = *inout_matrix;
    const float D = mat3x2_det(m);
    if (UNLIKELY(D == 0)) {
        return false;
    }
    const float inv = 1.0f / D;
    const float a = m.a * inv;
    const float b = m.b * inv;
    const float c = m.c * inv;
    const float d = m.d * inv;
    const float x = m.tx;
    const float y = m.ty;
    inout_matrix->a = d;
    inout_matrix->b = -b;
    inout_matrix->c = -c;
    inout_matrix->d = a;
    inout_matrix->tx = y * c - x * d;
    inout_matrix->ty = x * b - y * a;
    return true;
}

/** mat4x4 **/

mat4_t mat4_identity(void) {
    mat4_t m;
    m.data[0] = 1.0f;
    m.data[1] = 0.0f;
    m.data[2] = 0.0f;
    m.data[3] = 0.0f;
    m.data[4] = 0.0f;
    m.data[5] = 1.0f;
    m.data[6] = 0.0f;
    m.data[7] = 0.0f;
    m.data[8] = 0.0f;
    m.data[9] = 0.0f;
    m.data[10] = 1.0f;
    m.data[11] = 0.0f;
    m.data[12] = 0.0f;
    m.data[13] = 0.0f;
    m.data[14] = 0.0f;
    m.data[15] = 1.0f;
    return m;
}

mat4_t mat4_2d_transform(vec2_t pos, vec2_t scale, vec2_t skew) {
    mat4_t r = mat4_identity();
    mat2_t rot = mat2_scale_skew(scale, skew);
    r.data[0] = rot.a;
    r.data[1] = rot.c;
    r.data[4] = rot.b;
    r.data[5] = rot.d;
    r.data[12] = pos.x;
    r.data[13] = pos.y;
    return r;
}

// ZO meaning:
// "The near and far clip planes correspond to z normalized device coordinates of 0 and +1 respectively. (Direct3D clip volume definition)"
// from @glm library

// perspectiveRH_ZO
mat4_t mat4_perspective_rh(float fov_y, float aspect, float z_near, float z_far) {
    const float tan_half_fov_y = tanf(fov_y / 2.0f);
    mat4_t m;
    m.data[0] = 1.0f / (aspect * tan_half_fov_y);
    m.data[1] = 0.0f;
    m.data[2] = 0.0f;
    m.data[3] = 0.0f;
    m.data[4] = 0.0f;
    m.data[5] = 1.0f / tan_half_fov_y;
    m.data[6] = 0.0f;
    m.data[7] = 0.0f;
    m.data[8] = 0.0f;
    m.data[9] = 0.0f;
    m.data[10] = z_far / (z_near - z_far);
    m.data[11] = -1.0f;
    m.data[12] = 0.0f;
    m.data[13] = 0.0f;
    m.data[14] = -(z_far * z_near) / (z_far - z_near);
    m.data[15] = 0.0f;
    return m;
}

// perspectiveLH_ZO
mat4_t mat4_perspective_lh(float fov_y, float aspect, float z_near, float z_far) {
    mat4_t m;
    const float tan_half_fov_y = tanf(fov_y / 2.0f);
    m.data[0] = 1.0f / (aspect * tan_half_fov_y);
    m.data[1] = 0.0f;
    m.data[2] = 0.0f;
    m.data[3] = 0.0f;
    m.data[4] = 0.0f;
    m.data[5] = 1.0f / tan_half_fov_y;
    m.data[6] = 0.0f;
    m.data[7] = 0.0f;
    m.data[8] = 0.0f;
    m.data[9] = 0.0f;
    m.data[10] = z_far / (z_far - z_near);
    m.data[11] = 1.0f;
    m.data[12] = 0.0f;
    m.data[13] = 0.0f;
    m.data[14] = -(z_far * z_near) / (z_far - z_near);
    m.data[15] = 0.0f;
    return m;
}

// orthoLH_ZO
mat4_t mat4_orthographic_lh(float left, float right, float bottom, float top, float z_near, float z_far) {
    mat4_t m;
    m.data[0] = 2.0f / (right - left);
    m.data[1] = 0.0f;
    m.data[2] = 0.0f;
    m.data[3] = 0.0f;
    m.data[4] = 0.0f;
    m.data[5] = 2.0f / (top - bottom);
    m.data[6] = 0.0f;
    m.data[7] = 0.0f;
    m.data[8] = 0.0f;
    m.data[9] = 0.0f;
    m.data[10] = 1.0f / (z_far - z_near);
    m.data[11] = 0.0f;
    m.data[12] = -(right + left) / (right - left);
    m.data[13] = -(top + bottom) / (top - bottom);
    m.data[14] = -z_near / (z_far - z_near);
    m.data[15] = 1.0f;
    return m;
}

// orthoRH_ZO
mat4_t mat4_orthographic_rh(float left, float right, float bottom, float top, float z_near, float z_far) {
    mat4_t m;
    m.data[0] = 2.0f / (right - left);
    m.data[1] = 0.0f;
    m.data[2] = 0.0f;
    m.data[3] = 0.0f;
    m.data[4] = 0.0f;
    m.data[5] = 2.0f / (top - bottom);
    m.data[6] = 0.0f;
    m.data[7] = 0.0f;
    m.data[8] = 0.0f;
    m.data[9] = 0.0f;
    m.data[10] = -1.0f / (z_far - z_near);
    m.data[11] = 0.0f;
    m.data[12] = -(right + left) / (right - left);
    m.data[13] = -(top + bottom) / (top - bottom);
    m.data[14] = -z_near / (z_far - z_near);
    m.data[15] = 1.0f;
    return m;
}

// default:
// znear = -1,
// zfar = 1
mat4_t mat4_orthographic_2d(float x, float y, float w, float h, float z_near, float z_far) {
    EK_ASSERT_R2(w != 0 && h != 0);
    return mat4_orthographic_rh(x, x + w, y + h, y, z_near, z_far);
}

float vec2_dot(vec2_t a, vec2_t b) {
    return (a.x * b.x) + (a.y * b.y);
}

float vec3_dot(vec3_t a, vec3_t b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

float vec4_dot(vec4_t a, vec4_t b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

float vec2_distance_sqr(const vec2_t a, const vec2_t b) {
    return vec2_length_sqr(vec2_sub(a, b));
}

float vec3_distance_sqr(const vec3_t a, const vec3_t b) {
    return vec3_length_sqr(vec3_sub(a, b));
}

float vec4_distance_sqr(const vec4_t a, const vec4_t b) {
    return vec4_length_sqr(vec4_sub(a, b));
}

float vec2_distance(vec2_t a, vec2_t b) {
    return vec2_length(vec2_sub(a, b));
}

float vec3_distance(vec3_t a, vec3_t b) {
    return vec3_length(vec3_sub(a, b));
}

float vec4_distance(vec4_t a, vec4_t b) {
    return vec4_length(vec4_sub(a, b));
}

float vec2_length_sqr(vec2_t a) {
    return a.x * a.x + a.y * a.y;
}

float vec3_length_sqr(vec3_t a) {
    return a.x * a.x + a.y * a.y + a.z * a.z;
}

float vec4_length_sqr(vec4_t a) {
    return a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w;
}

float vec2_length(vec2_t a) {
    return sqrtf(vec2_length_sqr(a));
}

float vec3_length(vec3_t a) {
    return sqrtf(vec3_length_sqr(a));
}

float vec4_length(vec4_t a) {
    return sqrtf(vec4_length_sqr(a));
}

vec2_t vec2_add(vec2_t a, vec2_t b) {
    return (vec2_t) {{a.x + b.x, a.y + b.y}};
}

vec3_t vec3_add(vec3_t a, vec3_t b) {
    return (vec3_t) {{a.x + b.x, a.y + b.y, a.z + b.z}};
}

vec4_t vec4_add(vec4_t a, vec4_t b) {
    return (vec4_t) {{a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w}};
}

vec2_t vec2_sub(vec2_t a, vec2_t b) {
    return (vec2_t) {{a.x - b.x, a.y - b.y}};
}

vec3_t vec3_sub(vec3_t a, vec3_t b) {
    return (vec3_t) {{a.x - b.x, a.y - b.y, a.z - b.z}};
}

vec4_t vec4_sub(vec4_t a, vec4_t b) {
    return (vec4_t) {{a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w}};
}

vec2_t vec2_neg(vec2_t a) {
    return (vec2_t) {{-a.x, -a.y}};
}

vec3_t vec3_neg(vec3_t a) {
    return (vec3_t) {{-a.x, -a.y, -a.z}};
}

vec4_t vec4_neg(vec4_t a) {
    return (vec4_t) {{-a.x, -a.y, -a.z, -a.w}};
}

vec3_t vec3_cross(vec3_t a, vec3_t b) {
    return (vec3_t) {
            .x = a.y * b.z - a.z * b.y,
            .y = a.z * b.x - a.x * b.z,
            .z = a.x * b.y - a.y * b.x
    };
}

vec2_t vec2_scale(vec2_t a, float s) {
    return (vec2_t) {{a.x * s, a.y * s}};
}

vec3_t vec3_scale(vec3_t a, float s) {
    return (vec3_t) {{a.x * s, a.y * s, a.z * s}};
}

vec4_t vec4_scale(vec4_t a, float s) {
    return (vec4_t) {{a.x * s, a.y * s, a.z * s, a.w * s}};
}

vec2_t vec2_mul(vec2_t a, vec2_t b) {
    vec2_t r;
    r.x = a.x * b.x;
    r.y = a.y * b.y;
    return r;
}

vec3_t vec3_mul(vec3_t a, vec3_t b) {
    vec3_t r;
    r.x = a.x * b.x;
    r.y = a.y * b.y;
    r.z = a.z * b.z;
    return r;
}

vec4_t vec4_mul(vec4_t a, vec4_t b) {
    vec4_t r;
    r.x = a.x * b.x;
    r.y = a.y * b.y;
    r.z = a.z * b.z;
    r.w = a.w * b.w;
    return r;
}

vec2_t vec2_normalize(vec2_t a) {
    vec2_t result = {};
    const float ls = vec2_length_sqr(a);
    if (ls != 0.0f) {
        result = vec2_scale(a, 1.0f / sqrtf(ls));
    }
    return result;
}

vec3_t vec3_normalize(vec3_t a) {
    vec3_t result = {};
    const float ls = vec3_length_sqr(a);
    if (ls != 0.0f) {
        result = vec3_scale(a, 1.0f / sqrtf(ls));
    }
    return result;
}

vec4_t vec4_normalize(vec4_t a) {
    vec4_t result = {};
    const float ls = vec4_length_sqr(a);
    if (ls != 0.0f) {
        result = vec4_scale(a, 1.0f / sqrtf(ls));
    }
    return result;
}

vec2_t vec2_transform(const vec2_t point, const mat3x2_t matrix) {
    return (vec2_t) {{
                             point.x * matrix.a + point.y * matrix.c + matrix.tx,
                             point.x * matrix.b + point.y * matrix.d + matrix.ty
                     }};
}

vec2_t vec2_perp(vec2_t v) {
    vec2_t r;
    r.x = -v.y;
    r.y = v.x;
    return r;
}

bool vec2_transform_inverse(vec2_t p, mat3x2_t m, vec2_t* out) {
    const float a = m.a;
    const float b = m.b;
    const float c = m.c;
    const float d = m.d;
    const float x = p.x - m.tx;
    const float y = p.y - m.ty;
    const float det = mat2_det(m.rot);
    if (UNLIKELY(det == 0.0f)) {
        return false;
    }
    out->x = (x * d - y * c) / det;
    out->y = (y * a - x * b) / det;
    return true;
}


float f32_lerp(float a, float b, float t) {
    return (1.0f - t) * a + t * b;
}

vec2_t vec2_lerp(const vec2_t a, const vec2_t b, float t) {
    const float inv = 1.0f - t;
    return (vec2_t) {{
                             inv * a.x + t * b.x,
                             inv * a.y + t * b.y,
                     }};
}

vec3_t vec3_lerp(const vec3_t a, const vec3_t b, float t) {
    const float inv = 1.0f - t;
    return (vec3_t) {{
                             inv * a.x + t * b.x,
                             inv * a.y + t * b.y,
                             inv * a.z + t * b.z,
                     }};
}

vec4_t vec4_lerp(const vec4_t a, const vec4_t b, float t) {
    const float inv = 1.0f - t;
    return (vec4_t) {{
                             inv * a.x + t * b.x,
                             inv * a.y + t * b.y,
                             inv * a.z + t * b.z,
                             inv * a.w + t * b.w,
                     }};
}

// TODO:  look at view matrix calc
mat4_t mat4_look_at_rh(const vec3_t eye, const vec3_t center, const vec3_t up) {
    const vec3_t f = vec3_normalize(vec3_sub(center, eye));
    const vec3_t s = vec3_normalize(vec3_cross(f, up));
    const vec3_t u = vec3_cross(s, f);

    mat4_t m;
    m.data[0] = s.x;
    m.data[1] = s.y;
    m.data[2] = s.z;
    m.data[3] = 0.0f;
    m.data[4] = u.x;
    m.data[5] = u.y;
    m.data[6] = u.z;
    m.data[7] = 0.0f;
    m.data[8] = -f.x;
    m.data[9] = -f.y;
    m.data[10] = -f.z;
    m.data[11] = 0.0f;
    m.data[12] = -vec3_dot(s, eye);
    m.data[13] = -vec3_dot(u, eye);
    m.data[14] = vec3_dot(f, eye);
    m.data[15] = 1.0f;
    return m;
}

mat4_t mat4_look_at_lh(const vec3_t eye, const vec3_t center, const vec3_t up) {
    const vec3_t f = vec3_normalize(vec3_sub(center, eye));
    const vec3_t s = vec3_normalize(vec3_cross(up, f));
    const vec3_t u = vec3_cross(f, s);

    mat4_t m;
    m.data[0] = s.x;
    m.data[1] = s.y;
    m.data[2] = s.z;
    m.data[3] = 0.0f;
    m.data[4] = u.x;
    m.data[5] = u.y;
    m.data[6] = u.z;
    m.data[7] = 0.0f;
    m.data[8] = f.x;
    m.data[9] = f.y;
    m.data[10] = f.z;
    m.data[11] = 0.0f;
    m.data[12] = -vec3_dot(s, eye);
    m.data[13] = -vec3_dot(u, eye);
    m.data[14] = -vec3_dot(f, eye);
    m.data[15] = 1.0f;
    return m;
}

mat4_t mat4_mul(const mat4_t l, const mat4_t r) {
    mat4_t m;
    m.m00 = r.m00 * l.m00 + r.m01 * l.m10 + r.m02 * l.m20 + r.m03 * l.m30;
    m.m01 = r.m00 * l.m01 + r.m01 * l.m11 + r.m02 * l.m21 + r.m03 * l.m31;
    m.m02 = r.m00 * l.m02 + r.m01 * l.m12 + r.m02 * l.m22 + r.m03 * l.m32;
    m.m03 = r.m00 * l.m03 + r.m01 * l.m13 + r.m02 * l.m23 + r.m03 * l.m33;
    m.m10 = r.m10 * l.m00 + r.m11 * l.m10 + r.m12 * l.m20 + r.m13 * l.m30;
    m.m11 = r.m10 * l.m01 + r.m11 * l.m11 + r.m12 * l.m21 + r.m13 * l.m31;
    m.m12 = r.m10 * l.m02 + r.m11 * l.m12 + r.m12 * l.m22 + r.m13 * l.m32;
    m.m13 = r.m10 * l.m03 + r.m11 * l.m13 + r.m12 * l.m23 + r.m13 * l.m33;
    m.m20 = r.m20 * l.m00 + r.m21 * l.m10 + r.m22 * l.m20 + r.m23 * l.m30;
    m.m21 = r.m20 * l.m01 + r.m21 * l.m11 + r.m22 * l.m21 + r.m23 * l.m31;
    m.m22 = r.m20 * l.m02 + r.m21 * l.m12 + r.m22 * l.m22 + r.m23 * l.m32;
    m.m23 = r.m20 * l.m03 + r.m21 * l.m13 + r.m22 * l.m23 + r.m23 * l.m33;
    m.m30 = r.m30 * l.m00 + r.m31 * l.m10 + r.m32 * l.m20 + r.m33 * l.m30;
    m.m31 = r.m30 * l.m01 + r.m31 * l.m11 + r.m32 * l.m21 + r.m33 * l.m31;
    m.m32 = r.m30 * l.m02 + r.m31 * l.m12 + r.m32 * l.m22 + r.m33 * l.m32;
    m.m33 = r.m30 * l.m03 + r.m31 * l.m13 + r.m32 * l.m23 + r.m33 * l.m33;
    return m;
}

mat4_t mat4_mul_mat3x2(mat4_t l, mat3x2_t r) {
    mat4_t m;
    m.m00 = r.a * l.m00 + r.b * l.m10;
    m.m01 = r.a * l.m01 + r.b * l.m11;
    m.m02 = r.a * l.m02 + r.b * l.m12;
    m.m03 = r.a * l.m03 + r.b * l.m13;
    m.m10 = r.c * l.m00 + r.d * l.m10;
    m.m11 = r.c * l.m01 + r.d * l.m11;
    m.m12 = r.c * l.m02 + r.d * l.m12;
    m.m13 = r.c * l.m03 + r.d * l.m13;
    m.m20 = l.data[8];
    m.m21 = l.data[9];
    m.m22 = l.data[10];
    m.m23 = l.data[11];
    m.m30 = r.tx * l.m00 + r.ty * l.m10 + l.data[12];
    m.m31 = r.tx * l.m01 + r.ty * l.m11 + l.data[13];
    m.m32 = r.tx * l.m02 + r.ty * l.m12 + l.data[14];
    m.m33 = r.tx * l.m03 + r.ty * l.m13 + l.data[15];
    return m;
}

static vec4_t mat4_row(const mat4_t* m, unsigned i) {
    vec4_t r;
    r.x = m->data[i];
    r.y = m->data[4 + i];
    r.z = m->data[8 + i];
    r.w = m->data[12 + i];
    return r;
}

// TODO: expose
void pmat4_scale(float m[16], float scale) {
    for (unsigned i = 0; i < 16; ++i) {
        m[i] *= scale;
    }
}

mat4_t mat4_inverse(mat4_t m) {

    float coef_00 = m.m22 * m.m33 - m.m32 * m.m23;
    float coef_02 = m.m12 * m.m33 - m.m32 * m.m13;
    float coef_03 = m.m12 * m.m23 - m.m22 * m.m13;
    float coef_04 = m.m21 * m.m33 - m.m31 * m.m23;
    float coef_06 = m.m11 * m.m33 - m.m31 * m.m13;
    float coef_07 = m.m11 * m.m23 - m.m21 * m.m13;
    float coef_08 = m.m21 * m.m32 - m.m31 * m.m22;
    float coef_10 = m.m11 * m.m32 - m.m31 * m.m12;
    float coef_11 = m.m11 * m.m22 - m.m21 * m.m12;
    float coef_12 = m.m20 * m.m33 - m.m30 * m.m23;
    float coef_14 = m.m10 * m.m33 - m.m30 * m.m13;
    float coef_15 = m.m10 * m.m23 - m.m20 * m.m13;
    float coef_16 = m.m20 * m.m32 - m.m30 * m.m22;
    float coef_18 = m.m10 * m.m32 - m.m30 * m.m12;
    float coef_19 = m.m10 * m.m22 - m.m20 * m.m12;
    float coef_20 = m.m20 * m.m31 - m.m30 * m.m21;
    float coef_22 = m.m10 * m.m31 - m.m30 * m.m11;
    float coef_23 = m.m10 * m.m21 - m.m20 * m.m11;

    vec4_t fac_0 = vec4(coef_00, coef_00, coef_02, coef_03);
    vec4_t fac_1 = vec4(coef_04, coef_04, coef_06, coef_07);
    vec4_t fac_2 = vec4(coef_08, coef_08, coef_10, coef_11);
    vec4_t fac_3 = vec4(coef_12, coef_12, coef_14, coef_15);
    vec4_t fac_4 = vec4(coef_16, coef_16, coef_18, coef_19);
    vec4_t fac_5 = vec4(coef_20, coef_20, coef_22, coef_23);

    vec4_t v_0 = vec4(m.m10, m.m00, m.m00, m.m00);
    vec4_t v_1 = vec4(m.m11, m.m01, m.m01, m.m01);
    vec4_t v_2 = vec4(m.m12, m.m02, m.m02, m.m02);
    vec4_t v_3 = vec4(m.m13, m.m03, m.m03, m.m03);

    vec4_t inv_0 = vec4_add(vec4_sub(vec4_mul(v_1, fac_0), vec4_mul(v_2, fac_1)), vec4_mul(v_3, fac_2));
    vec4_t inv_1 = vec4_add(vec4_sub(vec4_mul(v_0, fac_0), vec4_mul(v_2, fac_3)), vec4_mul(v_3, fac_4));
    vec4_t inv_2 = vec4_add(vec4_sub(vec4_mul(v_0, fac_1), vec4_mul(v_1, fac_3)), vec4_mul(v_3, fac_5));
    vec4_t inv_3 = vec4_add(vec4_sub(vec4_mul(v_0, fac_2), vec4_mul(v_1, fac_4)), vec4_mul(v_2, fac_5));

    vec4_t sign_a = vec4(+1, -1, +1, -1);
    vec4_t sign_b = vec4(-1, +1, -1, +1);
    mat4_t inv_m;
    inv_m.columns[0] = vec4_mul(inv_0, sign_a);
    inv_m.columns[1] = vec4_mul(inv_1, sign_b);
    inv_m.columns[2] = vec4_mul(inv_2, sign_a);
    inv_m.columns[3] = vec4_mul(inv_3, sign_b);

    // mult components inv_m.col(0) * m.row(0)
    vec4_t dot_0 = vec4_mul(m.columns[0], mat4_row(&inv_m, 0));
    float dot_1 = (dot_0.x + dot_0.y) + (dot_0.z + dot_0.w);
    EK_ASSERT(dot_1 != 0.0f);
    pmat4_scale(inv_m.data, 1.0f / dot_1);
    return inv_m;
}

vec3_t mat4_get_position(const mat4_t* m) {
    return vec3(
            m->columns[3].x,
            m->columns[3].y,
            m->columns[3].z
    );
}

/** cross multiplications mat vs vec **/

// Post-multiply
vec4_t mat4_mul_vec4(mat4_t m, vec4_t v) {
    return vec4(v.x * m.m00 + v.y * m.m01 + v.z * m.m02 + v.w * m.m03,
                v.x * m.m10 + v.y * m.m11 + v.z * m.m12 + v.w * m.m13,
                v.x * m.m20 + v.y * m.m21 + v.z * m.m22 + v.w * m.m23,
                v.x * m.m30 + v.y * m.m31 + v.z * m.m32 + v.w * m.m33);
}

vec3_t mat4_mul_vec3(mat4_t m, vec3_t v) {
    const float w = v.x * m.m30 + v.y * m.m31 + v.z * m.m32 + m.m33;
    return vec3(v.x * m.m00 + v.y * m.m01 + v.z * m.m02 + w * m.m03,
                v.x * m.m10 + v.y * m.m11 + v.z * m.m12 + w * m.m13,
                v.x * m.m20 + v.y * m.m21 + v.z * m.m22 + w * m.m23);
}

// Pre-multiply
vec4_t vec4_mul_mat4(vec4_t v, mat4_t m) {
    return vec4(v.x * m.m00 + v.y * m.m10 + v.z * m.m20 + v.w * m.m30,
                v.x * m.m01 + v.y * m.m11 + v.z * m.m21 + v.w * m.m31,
                v.x * m.m02 + v.y * m.m12 + v.z * m.m22 + v.w * m.m32,
                v.x * m.m03 + v.y * m.m13 + v.z * m.m23 + v.w * m.m33);
}

vec3_t vec3_mul_mat4(vec3_t v, mat4_t m) {
    return vec3(v.x * m.m00 + v.y * m.m10 + v.z * m.m20 + m.m30,
                v.x * m.m01 + v.y * m.m11 + v.z * m.m21 + m.m31,
                v.x * m.m02 + v.y * m.m12 + v.z * m.m22 + m.m32);
}

/** u8 functions **/

uint8_t unorm8_f32(float f) {
    EK_ASSERT(f >= 0.0f && f <= 1.0f);
    return (uint8_t) (f * 255.0f);
}

uint8_t unorm8_f32_clamped(float f) {
    if (UNLIKELY(f < 0.0f)) {
        return 0;
    } else if (UNLIKELY(f > 1.0f)) {
        return 255;
    }
    return (uint8_t) (f * 255.0f);
}

uint8_t u8_norm_mul(uint8_t a, uint8_t b) {
    return (uint8_t) (((uint32_t) (a) * b * 258u) >> 16u);
}

uint8_t u8_add_sat(uint8_t a, uint8_t b) {
    uint8_t c;
    return __builtin_add_overflow(a, b, &c) ? 0xFF : c;
}

rgba_t rgba_u32(uint32_t value) {
    return (rgba_t) {.value = value};
}

rgba_t rgba_4f(const float r, const float g, const float b, const float a) {
    EK_ASSERT_R2(r >= 0.0f && r <= 1.0f);
    EK_ASSERT_R2(g >= 0.0f && g <= 1.0f);
    EK_ASSERT_R2(b >= 0.0f && b <= 1.0f);
    EK_ASSERT_R2(a >= 0.0f && a <= 1.0f);
    return (rgba_t) {
            .r = (uint8_t) (r * 255.0f),
            .g = (uint8_t) (g * 255.0f),
            .b = (uint8_t) (b * 255.0f),
            .a = (uint8_t) (a * 255.0f),
    };
}

rgba_t rgba_vec4(const vec4_t rgba) {
    return rgba_4f(rgba.x, rgba.y, rgba.z, rgba.w);
}

rgba_t rgba_mul(const rgba_t color, const rgba_t multiplier) {
    return (rgba_t) {
            .r = u8_norm_mul(color.r, multiplier.r),
            .g = u8_norm_mul(color.g, multiplier.g),
            .b = u8_norm_mul(color.b, multiplier.b),
            .a = u8_norm_mul(color.a, multiplier.a)
    };
}

rgba_t rgba_scale(const rgba_t color, const uint8_t multiplier) {
    return (rgba_t) {
            .r = u8_norm_mul(color.r, multiplier),
            .g = u8_norm_mul(color.g, multiplier),
            .b = u8_norm_mul(color.b, multiplier),
            .a = u8_norm_mul(color.a, multiplier)
    };
}

rgba_t rgba_add(const rgba_t color, const rgba_t add) {
    return (rgba_t) {
            .r = u8_add_sat(color.r, add.r),
            .g = u8_add_sat(color.g, add.g),
            .b = u8_add_sat(color.b, add.b),
            .a = u8_add_sat(color.a, add.a)
    };
}

rgba_t rgba_lerp(const rgba_t a, const rgba_t b, const float t) {
    const uint32_t r = (uint32_t) (t * 1024);
    const uint32_t ri = 1024u - r;
    return (rgba_t) {
            .r = (uint8_t) ((ri * a.r + r * b.r) >> 10u),
            .g = (uint8_t) ((ri * a.g + r * b.g) >> 10u),
            .b = (uint8_t) ((ri * a.b + r * b.b) >> 10u),
            .a = (uint8_t) ((ri * a.a + r * b.a) >> 10u)
    };
}

rgba_t rgba_alpha_scale_f(rgba_t color, const float alpha_multiplier) {
    color.a = (uint8_t) ((float) color.a * alpha_multiplier);
    return color;
}

color_mod_t color_mod_identity() {
    color_mod_t result;
    result.scale.value = 0xFFFFFFFFu;
    result.offset.value = 0;
    return result;
}

rgba_t color_mod_get_offset(rgba_t base_scale, rgba_t offset) {
    return (rgba_t) {
            .r = u8_norm_mul(offset.r, base_scale.r),
            .g = u8_norm_mul(offset.g, base_scale.g),
            .b = u8_norm_mul(offset.b, base_scale.b),
            .a = offset.a
    };
}

void color_mod_add(color_mod_t* color, rgba_t offset) {
    if (offset.value != 0) {
        color->offset = rgba_add(color->offset, color_mod_get_offset(color->scale, offset));
    }
}

void color_mod_concat(color_mod_t* color, const rgba_t scale, const rgba_t offset) {
    if (offset.value != 0) {
        color->offset = rgba_add(color->offset, color_mod_get_offset(color->scale, offset));
    }
    if (scale.value != 0xFFFFFFFF) {
        color->scale = rgba_mul(color->scale, scale);
    }
}

void color_mod_mul(color_mod_t* out, color_mod_t l, color_mod_t r) {
    out->scale = (~r.scale.value) != 0 ? rgba_mul(l.scale, r.scale) : l.scale;
    out->offset = r.offset.value != 0 ? rgba_add(l.offset, color_mod_get_offset(l.scale, r.offset)) : l.offset;
}

rect_t rect(float x, float y, float w, float h) {
    return (rect_t) {{x, y, w, h}};
}

rect_t recti_to_rect(const recti_t irc) {
    return (rect_t) {{
                             (float) irc.x,
                             (float) irc.y,
                             (float) irc.w,
                             (float) irc.h
                     }};
}

rect_t rect_size(const vec2_t size) {
    return (rect_t) {{0, 0, size.x, size.y}};
}

rect_t rect_wh(const float w, const float h) {
    return (rect_t) {{0, 0, w, h}};
}

rect_t rect_minmax(const vec2_t min, const vec2_t max) {
    return (rect_t) {{min.x, min.y, max.x - min.x, max.y - min.y}};
}

rect_t rect_01(void) {
    return (rect_t) {{0, 0, 1, 1}};
}

vec2_t vec2(float x, float y) {
    return (vec2_t) {{x, y}};
}

vec3_t vec3(float x, float y, float z) {
    return (vec3_t) {{x, y, z}};
}

vec4_t vec4(float x, float y, float z, float w) {
    return (vec4_t) {{x, y, z, w}};
}

vec4_t vec4_rgba(const rgba_t rgba) {
    vec4_t v;
    v.x = (float) rgba.r / 255.0f;
    v.y = (float) rgba.g / 255.0f;
    v.z = (float) rgba.b / 255.0f;
    v.w = (float) rgba.a / 255.0f;
    return v;
}

vec2_t vec2_max(const vec2_t a, const vec2_t b) {
    return (vec2_t) {{MAX(a.x, b.x), MAX(a.y, b.y)}};
}

vec3_t vec3_max(const vec3_t a, const vec3_t b) {
    return (vec3_t) {{MAX(a.x, b.x), MAX(a.y, b.y), MAX(a.z, b.z)}};
}

vec4_t vec4_max(const vec4_t a, const vec4_t b) {
    return (vec4_t) {{MAX(a.x, b.x), MAX(a.y, b.y), MAX(a.z, b.z), MAX(a.w, b.w)}};
}

vec2_t vec2_min(const vec2_t a, const vec2_t b) {
    return (vec2_t) {{MIN(a.x, b.x), MIN(a.y, b.y)}};
}

vec3_t vec3_min(const vec3_t a, const vec3_t b) {
    return (vec3_t) {{MIN(a.x, b.x), MIN(a.y, b.y), MIN(a.z, b.z)}};
}

vec4_t vec4_min(const vec4_t a, const vec4_t b) {
    return (vec4_t) {{MIN(a.x, b.x), MIN(a.y, b.y), MIN(a.z, b.z), MIN(a.w, b.w)}};
}

vec2_t rect_rb(const rect_t rc) {
    return (vec2_t) {{rc.x + rc.w, rc.y + rc.h}};
}

vec2_t rect_center(rect_t rc) {
    return (vec2_t) {{rc.x + rc.w * 0.5f, rc.y + rc.h * 0.5f}};
}

rect_t rect_clamp_bounds(const rect_t a, const rect_t b) {
    return rect_minmax(vec2_max(a.position, b.position),
                       vec2_min(rect_rb(a), rect_rb(b)));
}

recti_t recti_clamp_bounds(const recti_t a, const recti_t b) {
    const int x0 = MAX(a.x, b.x);
    const int y0 = MAX(a.y, b.y);
    const int x1 = MIN(RECT_R(a), RECT_R(b));
    const int y1 = MIN(RECT_B(a), RECT_B(b));
    return (recti_t) {{x0, y0, x1 - x0, y1 - y0}};
}

rect_t rect_combine(const rect_t a, const rect_t b) {
    return rect_minmax(vec2_min(a.position, b.position),
                       vec2_max(rect_rb(a), rect_rb(b)));
}

recti_t recti_combine(const recti_t a, const recti_t b) {
    recti_t result;
    result.x = MIN(a.x, b.x);
    result.y = MIN(a.y, b.y);
    result.w = MAX(RECT_R(a), RECT_R(b)) - result.x;
    result.h = MAX(RECT_B(a), RECT_B(b)) - result.y;
    return result;
}

bool rect_overlaps(const rect_t a, const rect_t b) {
    return a.x <= RECT_R(b) && b.x <= RECT_R(a) && a.y <= RECT_B(b) && b.y <= RECT_B(a);
}

bool recti_overlaps(const recti_t a, const recti_t b) {
    return a.x <= RECT_R(b) && b.x <= RECT_R(a) && a.y <= RECT_B(b) && b.y <= RECT_B(a);
}

rect_t rect_scale(const rect_t a, const vec2_t s) {
    rect_t r = (rect_t) {{a.x * s.x, a.y * s.y, a.w * s.x, a.h * s.y}};
    if (r.w < 0.0f) {
        r.x += r.w;
        r.w = -r.w;
    }
    if (r.h < 0) {
        r.y += r.h;
        r.h = -r.h;
    }
    return r;
}

rect_t rect_scale_f(const rect_t a, const float s) {
    return rect_scale(a, vec2(s, s));
}

rect_t rect_translate(const rect_t a, const vec2_t t) {
    return (rect_t) {{a.x + t.x, a.y + t.y, a.w, a.h}};
}

rect_t rect_transform(const rect_t rc, const mat3x2_t matrix) {
    return brect_get_rect(
            brect_extend_transformed_rect(brect_inf(), rc, matrix)
    );
}

bool rect_contains(rect_t rc, vec2_t point) {
    return rc.x <= point.x &&
           point.x <= (rc.x + rc.w) &&
           rc.y <= point.y &&
           point.y <= (rc.y + rc.h);
}

bool rect_is_empty(rect_t a) {
    return a.w <= 0 || a.h <= 0;
}

rect_t rect_expand(rect_t a, float w) {
    const float w2 = 2.0f * w;
    return (rect_t) {{a.x - w, a.y - w, a.w + w2, a.h + w2}};
}

circle_t circle(float x, float y, float r) {
    return (circle_t) {.x = x, .y = y, .r = r};
}

brect_t brect_from_rect(rect_t rc) {
    brect_t br;
    br.x0 = rc.x;
    br.y0 = rc.y;
    br.x1 = rc.x + rc.w;
    br.y1 = rc.y + rc.h;
    return br;
}

brect_t brect_inf(void) {
    brect_t br;
    br.x0 = 1.0e24f;
    br.y0 = 1.0e24f;
    br.x1 = -1.0e24f;
    br.y1 = -1.0e24f;
    return br;
}

static brect_t brect_push(brect_t br, float e0, float e1, float e2, float e3) {
    if (e0 < br.data[0]) br.data[0] = e0;
    if (e1 < br.data[1]) br.data[1] = e1;
    if (e2 > br.data[2]) br.data[2] = e2;
    if (e3 > br.data[3]) br.data[3] = e3;
    return br;
}

brect_t brect_extend_circle(brect_t bb, const circle_t circle) {
    return brect_push(bb,
                      circle.x - circle.r,
                      circle.y - circle.r,
                      circle.x + circle.r,
                      circle.y + circle.r);
}

brect_t brect_extend_rect(brect_t br, const rect_t rc) {
    return brect_push(br, rc.x, rc.y, rc.x + rc.w, rc.y + rc.h);
}

brect_t brect_extend_point(brect_t br, const vec2_t point) {
    return brect_push(br, point.x, point.y, point.x, point.y);
}

brect_t brect_extend_transformed_rect(brect_t bb, const rect_t rc, const mat3x2_t matrix) {
    bb = brect_extend_point(bb, vec2_transform(vec2(rc.x, rc.y), matrix));
    bb = brect_extend_point(bb, vec2_transform(vec2(rc.x + rc.w, rc.y), matrix));
    bb = brect_extend_point(bb, vec2_transform(vec2(rc.x, rc.y + rc.h), matrix));
    bb = brect_extend_point(bb, vec2_transform(vec2(rc.x + rc.w, rc.y + rc.h), matrix));
    return bb;
}

rect_t brect_get_rect(const brect_t brc) {
    rect_t r;
    r.x = brc.x0;
    r.y = brc.y0;
    r.w = brc.x1 > brc.x0 ? (brc.x1 - brc.x0) : 0.0f;
    r.h = brc.y1 > brc.y0 ? (brc.y1 - brc.y0) : 0.0f;
    return r;
}

bool brect_is_empty(brect_t bb) {
    return bb.x1 > bb.x0 && bb.y1 > bb.y0;
}

bool almost_eq_f32(float a, float b, float eps) {
    EK_ASSERT(eps > 0.0f);
    return fabsf(a - b) < eps;
}

bool almost_eq_vec2(const vec2_t a, const vec2_t b, const float eps) {
    return almost_eq_f32(a.x, b.x, eps) &&
           almost_eq_f32(a.y, b.y, eps);
}

bool almost_eq_vec3(const vec3_t a, const vec3_t b, const float eps) {
    return almost_eq_f32(a.x, b.x, eps) &&
           almost_eq_f32(a.y, b.y, eps) &&
           almost_eq_f32(a.z, b.z, eps);
}

bool almost_eq_vec4(const vec4_t a, const vec4_t b, const float eps) {
    return almost_eq_f32(a.x, b.x, eps) &&
           almost_eq_f32(a.y, b.y, eps) &&
           almost_eq_f32(a.z, b.z, eps) &&
           almost_eq_f32(a.w, b.w, eps);
}

#ifdef __cplusplus
}
#endif
