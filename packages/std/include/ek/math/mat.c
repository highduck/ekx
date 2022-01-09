
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

mat2_t mat2_affine_inverse(mat2_t m) {
    float d = mat2_det(m);
    EK_ASSERT(d != 0);

    mat2_t r;
    r.m00 = m.m11 / d;
    r.m01 = m.m01 / d;
    r.m10 = m.m10 / d;
    r.m11 = m.m00 / d;
    return r;
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
    return mat4_d(1);
}

mat4_t mat4_d(float d) {
    mat4_t m;
    m.data[0] = d;
    m.data[1] = 0;
    m.data[2] = 0;
    m.data[3] = 0;
    m.data[4] = 0;
    m.data[5] = d;
    m.data[6] = 0;
    m.data[7] = 0;
    m.data[8] = 0;
    m.data[9] = 0;
    m.data[10] = d;
    m.data[11] = 0;
    m.data[12] = 0;
    m.data[13] = 0;
    m.data[14] = 0;
    m.data[15] = d;
    return m;
}

mat4_t mat4_mat3(mat3_t m) {
    mat4_t result;
    result.m00 = m.m00;
    result.m01 = m.m01;
    result.m02 = m.m02;
    result.m03 = 0;
    result.m10 = m.m10;
    result.m11 = m.m11;
    result.m12 = m.m12;
    result.m13 = 0;
    result.m20 = m.m20;
    result.m21 = m.m21;
    result.m22 = m.m22;
    result.m23 = 0;
    result.m30 = 0;
    result.m31 = 0;
    result.m32 = 0;
    result.m33 = 1;
    return result;
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
//
//        hmm_mat4 Result = HMM_PREFIX(Mat4)();
//
//        // See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml
//
//        float Cotangent = 1.0f / HMM_PREFIX(TanF)(FOV * (HMM_PI32 / 360.0f));
//
//        Result.Elements[0][0] = Cotangent / AspectRatio;
//        Result.Elements[1][1] = Cotangent;
//        Result.Elements[2][3] = -1.0f;
//        Result.Elements[2][2] = (Near + Far) / (Near - Far);
//        Result.Elements[3][2] = (2.0f * Near * Far) / (Near - Far);
//        Result.Elements[3][3] = 0.0f;



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

vec2_t vec2_transform(const vec2_t point, const mat3x2_t matrix) {
    return (vec2_t) {{
                             point.x * matrix.a + point.y * matrix.c + matrix.tx,
                             point.x * matrix.b + point.y * matrix.d + matrix.ty
                     }};
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

// TODO:  look at view matrix calc
mat4_t mat4_look_at_rh(const vec3_t eye, const vec3_t center, const vec3_t up) {
    const vec3_t f = vec3_normalize(vec3_sub(center, eye));
    const vec3_t s = vec3_normalize(vec3_cross(f, up));
    const vec3_t u = vec3_cross(s, f);

    mat4_t m = mat4_identity();
    m.m00 = s.x;
    m.m10 = s.y;
    m.m20 = s.z;
    m.m01 = u.x;
    m.m11 = u.y;
    m.m21 = u.z;
    m.m02 = -f.x;
    m.m12 = -f.y;
    m.m22 = -f.z;
    m.m30 = -vec3_dot(s, eye);
    m.m31 = -vec3_dot(u, eye);
    m.m32 = vec3_dot(f, eye);
    return m;
}

mat4_t mat4_look_at_lh(const vec3_t eye, const vec3_t center, const vec3_t up) {
    const vec3_t f = vec3_normalize(vec3_sub(center, eye));
    const vec3_t s = vec3_normalize(vec3_cross(up, f));
    const vec3_t u = vec3_cross(f, s);

    mat4_t m = mat4_identity();
    m.m00 = s.x;
    m.m10 = s.y;
    m.m20 = s.z;
    m.m01 = u.x;
    m.m11 = u.y;
    m.m21 = u.z;
    m.m02 = f.x;
    m.m12 = f.y;
    m.m22 = f.z;
    m.m30 = -vec3_dot(s, eye);
    m.m31 = -vec3_dot(u, eye);
    m.m32 = -vec3_dot(f, eye);
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

mat3_t mat4_get_mat3(mat4_t* m) {
    mat3_t result;
    result.m00 = m->m00;
    result.m10 = m->m10;
    result.m20 = m->m20;
    result.m01 = m->m01;
    result.m11 = m->m11;
    result.m21 = m->m21;
    result.m02 = m->m02;
    result.m12 = m->m12;
    result.m22 = m->m22;
    return result;
}

mat3_t mat3_inverse(mat3_t m) {
    float det = +m.m00 * m.m11 * m.m22 - m.m21 * m.m12
                - m.m10 * m.m01 * m.m22 - m.m21 * m.m02
                + m.m20 * m.m01 * m.m12 - m.m11 * m.m02;
    EK_ASSERT(det != 0);
    float inv_det = 1.0f / det;

    mat3_t result;
    result.m00 = +(m.m11 * m.m22 - m.m21 * m.m12) * inv_det;
    result.m10 = -(m.m10 * m.m22 - m.m20 * m.m12) * inv_det;
    result.m20 = +(m.m10 * m.m21 - m.m20 * m.m11) * inv_det;
    result.m01 = -(m.m01 * m.m22 - m.m21 * m.m02) * inv_det;
    result.m11 = +(m.m00 * m.m22 - m.m20 * m.m02) * inv_det;
    result.m21 = -(m.m00 * m.m21 - m.m20 * m.m01) * inv_det;
    result.m02 = +(m.m01 * m.m12 - m.m11 * m.m02) * inv_det;
    result.m12 = -(m.m00 * m.m12 - m.m10 * m.m02) * inv_det;
    result.m22 = +(m.m00 * m.m11 - m.m10 * m.m01) * inv_det;

    return result;
}

mat3_t mat3_transpose(mat3_t m) {
    mat3_t result;
    result.m00 = m.m00;
    result.m01 = m.m10;
    result.m02 = m.m20;
    result.m10 = m.m01;
    result.m11 = m.m11;
    result.m12 = m.m21;
    result.m20 = m.m02;
    result.m21 = m.m12;
    result.m22 = m.m22;
    return result;
}


mat4_t mat4_transpose(mat4_t m) {
    mat4_t result;
    result.m00 = m.m00;
    result.m01 = m.m10;
    result.m02 = m.m20;
    result.m03 = m.m30;
    result.m10 = m.m01;
    result.m11 = m.m11;
    result.m12 = m.m21;
    result.m13 = m.m31;
    result.m20 = m.m02;
    result.m21 = m.m12;
    result.m22 = m.m22;
    result.m23 = m.m32;
    result.m30 = m.m03;
    result.m31 = m.m13;
    result.m32 = m.m23;
    result.m33 = m.m33;
    return result;
}

vec3_t mat4_get_position(const mat4_t* m) {
    return m->columns[3].xyz;
}

mat4_t mat4_rotate(const mat4_t m, float angle, const vec3_t v) {
    const float c = cosf(angle);
    const float s = sinf(angle);

    vec3_t axis = vec3_normalize(v);
    vec3_t temp = vec3_scale(axis, 1 - c);

    mat4_t r = mat4_identity();
    r.m00 = c + temp.x * axis.x;
    r.m01 = temp.x * axis.y + s * axis.z;
    r.m02 = temp.x * axis.z - s * axis.y;
    r.m10 = temp.y * axis.x - s * axis.z;
    r.m11 = c + temp.y * axis.y;
    r.m12 = temp.y * axis.z + s * axis.x;
    r.m20 = temp.z * axis.x + s * axis.y;
    r.m21 = temp.z * axis.y - s * axis.x;
    r.m22 = c + temp.z * axis.z;

    mat4_t result;
    result.columns[0] = vec4_add(vec4_add(vec4_scale(m.columns[0], r.m00), vec4_scale(m.columns[1], r.m01)),
                                 vec4_scale(m.columns[2], r.m02));
    result.columns[1] = vec4_add(vec4_add(vec4_scale(m.columns[0], r.m10), vec4_scale(m.columns[1], r.m11)),
                                 vec4_scale(m.columns[2], r.m12));
    result.columns[2] = vec4_add(vec4_add(vec4_scale(m.columns[0], r.m20), vec4_scale(m.columns[1], r.m21)),
                                 vec4_scale(m.columns[2], r.m22));
    result.columns[3] = m.columns[3];
    return result;
}

mat4_t mat4_rotation_transform_xzy(const vec3_t v) {
    const float c1 = cosf(-v.x);
    const float c2 = cosf(-v.y);
    const float c3 = cosf(-v.z);
    const float s1 = sinf(-v.x);
    const float s2 = sinf(-v.y);
    const float s3 = sinf(-v.z);

    mat4_t m;
    m.m00 = c2 * c3;
    m.m01 = -c1 * s3 + s1 * s2 * c3;
    m.m02 = s1 * s3 + c1 * s2 * c3;
    m.m03 = 0;
    m.m10 = c2 * s3;
    m.m11 = c1 * c3 + s1 * s2 * s3;
    m.m12 = -s1 * c3 + c1 * s2 * s3;
    m.m13 = 0;
    m.m20 = -s2;
    m.m21 = s1 * c2;
    m.m22 = c1 * c2;
    m.m23 = 0;
    m.m30 = 0;
    m.m31 = 0;
    m.m32 = 0;
    m.m33 = 1;
    return m;
}

mat4_t mat4_translate_transform(const vec3_t translation) {
    mat4_t m = mat4_identity();
    m.columns[3].xyz = translation;
    return m;
}

mat4_t mat4_scale_transform(const vec3_t scale) {
    mat4_t m = mat4_identity();
    m.m00 = scale.x;
    m.m11 = scale.y;
    m.m22 = scale.z;
    return m;
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

#ifdef __cplusplus
}
#endif
