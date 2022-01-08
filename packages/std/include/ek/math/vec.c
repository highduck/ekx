#ifdef __cplusplus
extern "C" {
#endif

vec2i_t vec2i(int x, int y) {
    return (vec2i_t) {{x, y}};
}

vec3i_t vec3i(int x, int y, int z) {
    return (vec3i_t) {{x, y, z}};
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

vec4_t vec4_v(vec3_t v, float w) {
    return vec4(v.x, v.y, v.z, w);
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
    return vec2_dot(a, a);
}

float vec3_length_sqr(vec3_t a) {
    return vec3_dot(a, a);
}

float vec4_length_sqr(vec4_t a) {
    return vec4_dot(a, a);
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
    const float ls = vec2_length(a);
    if (ls != 0.0f) {
        result = vec2_scale(a, 1.0f / ls);
    }
    return result;
}

vec3_t vec3_normalize(vec3_t a) {
    vec3_t result = {};
    const float ls = vec3_length(a);
    if (ls != 0.0f) {
        result = vec3_scale(a, 1.0f / ls);
    }
    return result;
}

vec4_t vec4_normalize(vec4_t a) {
    vec4_t result = {};
    const float ls = vec4_length(a);
    if (ls != 0.0f) {
        result = vec4_scale(a, 1.0f / ls);
    }
    return result;
}


vec2_t vec2_perp(vec2_t v) {
    vec2_t r;
    r.x = -v.y;
    r.y = v.x;
    return r;
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

// TODO: reflect_*
#define VEC_T_REFLECT_IMPL(T) T##_t T##_reflect(T##_t dir, T##_t normal) { \
    return T##_sub(dir, T##_scale(normal, T##_dot(dir, normal)));                 \
}

VEC_T_REFLECT_IMPL(vec2)

VEC_T_REFLECT_IMPL(vec3)

#ifdef __cplusplus
}
#endif
