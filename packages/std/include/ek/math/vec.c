#ifdef __cplusplus
extern "C" {
#endif

ivec2_t ivec2(int x, int y) {
    return (ivec2_t) {{x, y}};
}

ivec3_t ivec3(int x, int y, int z) {
    return (ivec3_t) {{x, y, z}};
}

vec2_t vec2(float x, float y) {
    return (vec2_t) {{x, y}};
}

vec2_t vec2_cs(float angle) {
    return vec2(cosf(angle), sinf(angle));
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

vec3_t vec3_v(vec2_t v, float z) {
    return vec3(v.x, v.y, z);
}

vec2_t max_vec2(vec2_t a, vec2_t b) {
    return (vec2_t) {{MAX(a.x, b.x), MAX(a.y, b.y)}};
}

vec3_t max_vec3(vec3_t a, vec3_t b) {
    return (vec3_t) {{MAX(a.x, b.x), MAX(a.y, b.y), MAX(a.z, b.z)}};
}

vec4_t max_vec4(vec4_t a, vec4_t b) {
    return (vec4_t) {{MAX(a.x, b.x), MAX(a.y, b.y), MAX(a.z, b.z), MAX(a.w, b.w)}};
}

vec2_t min_vec2(vec2_t a, vec2_t b) {
    return (vec2_t) {{MIN(a.x, b.x), MIN(a.y, b.y)}};
}

vec3_t min_vec3(vec3_t a, vec3_t b) {
    return (vec3_t) {{MIN(a.x, b.x), MIN(a.y, b.y), MIN(a.z, b.z)}};
}

vec4_t min_vec4(vec4_t a, vec4_t b) {
    return (vec4_t) {{MIN(a.x, b.x), MIN(a.y, b.y), MIN(a.z, b.z), MIN(a.w, b.w)}};
}

float dot_vec2(vec2_t a, vec2_t b) {
    return (a.x * b.x) + (a.y * b.y);
}

float dot_vec3(vec3_t a, vec3_t b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

float dot_vec4(vec4_t a, vec4_t b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

float distance_sqr_vec2(const vec2_t a, const vec2_t b) {
    return length_sqr_vec2(sub_vec2(a, b));
}

float distance_sqr_vec3(const vec3_t a, const vec3_t b) {
    return length_sqr_vec3(sub_vec3(a, b));
}

float distance_sqr_vec4(const vec4_t a, const vec4_t b) {
    return length_sqr_vec4(sub_vec4(a, b));
}

float distance_vec2(vec2_t a, vec2_t b) {
    return length_vec2(sub_vec2(a, b));
}

float distance_vec3(vec3_t a, vec3_t b) {
    return length_vec3(sub_vec3(a, b));
}

float distance_vec4(vec4_t a, vec4_t b) {
    return length_vec4(sub_vec4(a, b));
}

float length_sqr_vec2(vec2_t a) {
    return dot_vec2(a, a);
}

float length_sqr_vec3(vec3_t a) {
    return dot_vec3(a, a);
}

float length_sqr_vec4(vec4_t a) {
    return dot_vec4(a, a);
}

float length_vec2(vec2_t a) {
    return sqrtf(length_sqr_vec2(a));
}

float length_vec3(vec3_t a) {
    return sqrtf(length_sqr_vec3(a));
}

float length_vec4(vec4_t a) {
    return sqrtf(length_sqr_vec4(a));
}

vec2_t add_vec2(vec2_t a, vec2_t b) {
    return (vec2_t) {{a.x + b.x, a.y + b.y}};
}

vec3_t add_vec3(vec3_t a, vec3_t b) {
    return (vec3_t) {{a.x + b.x, a.y + b.y, a.z + b.z}};
}

vec4_t add_vec4(vec4_t a, vec4_t b) {
    return (vec4_t) {{a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w}};
}

vec2_t sub_vec2(vec2_t a, vec2_t b) {
    return (vec2_t) {{a.x - b.x, a.y - b.y}};
}

vec3_t sub_vec3(vec3_t a, vec3_t b) {
    return (vec3_t) {{a.x - b.x, a.y - b.y, a.z - b.z}};
}

vec4_t sub_vec4(vec4_t a, vec4_t b) {
    return (vec4_t) {{a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w}};
}

vec2_t neg_vec2(vec2_t a) {
    return (vec2_t) {{-a.x, -a.y}};
}

vec3_t neg_vec3(vec3_t a) {
    return (vec3_t) {{-a.x, -a.y, -a.z}};
}

vec4_t neg_vec4(vec4_t a) {
    return (vec4_t) {{-a.x, -a.y, -a.z, -a.w}};
}

vec3_t cross_vec3(vec3_t a, vec3_t b) {
    return (vec3_t) {
            .x = a.y * b.z - a.z * b.y,
            .y = a.z * b.x - a.x * b.z,
            .z = a.x * b.y - a.y * b.x
    };
}

vec2_t scale_vec2(vec2_t a, float s) {
    return (vec2_t) {{a.x * s, a.y * s}};
}

vec3_t scale_vec3(vec3_t a, float s) {
    return (vec3_t) {{a.x * s, a.y * s, a.z * s}};
}

vec4_t scale_vec4(vec4_t a, float s) {
    return (vec4_t) {{a.x * s, a.y * s, a.z * s, a.w * s}};
}

vec2_t add_scale_vec2(vec2_t a, vec2_t b, float s) {
    return (vec2_t) {{
                             a.x + b.x * s,
                             a.y + b.y * s,
                     }};
}

vec3_t add_scale_vec3(vec3_t a, vec3_t b, float s) {
    return (vec3_t) {{
                             a.x + b.x * s,
                             a.y + b.y * s,
                             a.z + b.z * s,
                     }};
}

vec4_t add_scale_vec4(vec4_t a, vec4_t b, float s) {
    return (vec4_t) {{
                             a.x + b.x * s,
                             a.y + b.y * s,
                             a.z + b.z * s,
                             a.w + b.w * s,
                     }};
}

vec2_t mul_vec2(vec2_t a, vec2_t b) {
    vec2_t r;
    r.x = a.x * b.x;
    r.y = a.y * b.y;
    return r;
}

vec3_t mul_vec3(vec3_t a, vec3_t b) {
    vec3_t r;
    r.x = a.x * b.x;
    r.y = a.y * b.y;
    r.z = a.z * b.z;
    return r;
}

vec4_t mul_vec4(vec4_t a, vec4_t b) {
    vec4_t r;
    r.x = a.x * b.x;
    r.y = a.y * b.y;
    r.z = a.z * b.z;
    r.w = a.w * b.w;
    return r;
}

vec2_t normalize_vec2(vec2_t a) {
    vec2_t result = {};
    const float ls = length_vec2(a);
    if (ls != 0.0f) {
        result = scale_vec2(a, 1.0f / ls);
    }
    return result;
}

vec3_t normalize_vec3(vec3_t a) {
    vec3_t result = {};
    const float ls = length_vec3(a);
    if (ls != 0.0f) {
        result = scale_vec3(a, 1.0f / ls);
    }
    return result;
}

vec4_t normalize_vec4(vec4_t a) {
    vec4_t result = {};
    const float ls = length_vec4(a);
    if (ls != 0.0f) {
        result = scale_vec4(a, 1.0f / ls);
    }
    return result;
}


vec2_t perp_vec2(vec2_t v) {
    vec2_t r;
    r.x = -v.y;
    r.y = v.x;
    return r;
}

vec2_t lerp_vec2(vec2_t a, vec2_t b, float t) {
    const float inv = 1.0f - t;
    return (vec2_t) {{
                             inv * a.x + t * b.x,
                             inv * a.y + t * b.y,
                     }};
}

vec3_t lerp_vec3(vec3_t a, vec3_t b, float t) {
    const float inv = 1.0f - t;
    return (vec3_t) {{
                             inv * a.x + t * b.x,
                             inv * a.y + t * b.y,
                             inv * a.z + t * b.z,
                     }};
}

vec4_t lerp_vec4(vec4_t a, vec4_t b, float t) {
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
#define VEC_T_REFLECT_IMPL(T) T##_t reflect_##T(T##_t dir, T##_t normal) { \
    return sub_##T(dir, scale_##T(normal, dot_##T(dir, normal)));          \
}

VEC_T_REFLECT_IMPL(vec2)

VEC_T_REFLECT_IMPL(vec3)

#ifdef __cplusplus
}
#endif
