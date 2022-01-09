#ifndef EK_MATH_VEC_H
#define EK_MATH_VEC_H

#ifdef __cplusplus
extern "C" {
#endif

typedef union vec2_t {
    struct {
        float x;
        float y;
    };
    float data[2];
#ifdef __cplusplus

    inline float& operator[](const int index) { return data[index]; }

#endif
} vec2_t;

typedef union vec3_t {
    struct {
        float x;
        float y;
        float z;
    };
    float data[3];
    struct {
        vec2_t xy;
        float _z;
    };
#ifdef __cplusplus

    inline float& operator[](const int index) { return data[index]; }

#endif
} vec3_t;

typedef union vec4_t {
    struct {
        float x, y, z, w;
    };
    float data[4];
    struct {
        vec3_t xyz;
        float _w;
    };
    struct {
        vec2_t xy;
        vec2_t zw;
    };
    struct {
        float r, g, b, a;
    };
    struct {
        float hue;
        float saturation;
        float value;
        float alpha;
    };
#ifdef __cplusplus

    inline float& operator[](const int index) { return data[index]; }

#endif
} vec4_t;

typedef union vec2i_t {
    struct {
        int x;
        int y;
    };
    int data[2];
#ifdef __cplusplus

    inline int& operator[](const int index) { return data[index]; }

#endif
} vec2i_t;

typedef union vec3i_t {
    struct {
        int x;
        int y;
        int z;
    };
    int data[3];
#ifdef __cplusplus

    inline int& operator[](const int index) { return data[index]; }

#endif
} vec3i_t;

typedef union vec4i_t {
    struct {
        int x;
        int y;
        int z;
        int w;
    };
    int data[4];
    struct {
        vec3i_t xyz;
        int _w;
    };
    struct {
        vec2i_t xy;
        vec2i_t zw;
    };
#ifdef __cplusplus

    inline int& operator[](const int index) { return data[index]; }

#endif
} vec4i_t;

vec2i_t vec2i(int x, int y);
vec3i_t vec3i(int x, int y, int z);

vec2_t vec2(float x, float y);
vec3_t vec3(float x, float y, float z);
vec4_t vec4(float x, float y, float z, float w);
vec3_t vec3_v(vec2_t v, float z);
vec4_t vec4_v(vec3_t v, float w);

vec2_t max_vec2(vec2_t a, vec2_t b);
vec3_t max_vec3(vec3_t a, vec3_t b);
vec4_t max_vec4(vec4_t a, vec4_t b);

vec2_t min_vec2(vec2_t a, vec2_t b);
vec3_t min_vec3(vec3_t a, vec3_t b);
vec4_t min_vec4(vec4_t a, vec4_t b);

float dot_vec2(vec2_t a, vec2_t b);
float dot_vec3(vec3_t a, vec3_t b);
float dot_vec4(vec4_t a, vec4_t b);

float distance_sqr_vec2(vec2_t a, vec2_t b);
float distance_sqr_vec3(vec3_t a, vec3_t b);
float distance_sqr_vec4(vec4_t a, vec4_t b);

float distance_vec2(vec2_t a, vec2_t b);
float distance_vec3(vec3_t a, vec3_t b);
float distance_vec4(vec4_t a, vec4_t b);

float length_sqr_vec2(vec2_t a);
float length_sqr_vec3(vec3_t a);
float length_sqr_vec4(vec4_t a);

float length_vec2(vec2_t a);
float length_vec3(vec3_t a);
float length_vec4(vec4_t a);

vec2_t add_vec2(vec2_t a, vec2_t b);
vec3_t add_vec3(vec3_t a, vec3_t b);
vec4_t add_vec4(vec4_t a, vec4_t b);

vec2_t sub_vec2(vec2_t a, vec2_t b);
vec3_t sub_vec3(vec3_t a, vec3_t b);
vec4_t sub_vec4(vec4_t a, vec4_t b);

vec2_t neg_vec2(vec2_t a);
vec3_t neg_vec3(vec3_t a);
vec4_t neg_vec4(vec4_t a);

vec3_t cross_vec3(vec3_t a, vec3_t b);

vec2_t scale_vec2(vec2_t a, float s);
vec3_t scale_vec3(vec3_t a, float s);
vec4_t scale_vec4(vec4_t a, float s);

vec2_t mul_vec2(vec2_t a, vec2_t b);
vec3_t mul_vec3(vec3_t a, vec3_t b);
vec4_t mul_vec4(vec4_t a, vec4_t b);

vec2_t normalize_vec2(vec2_t a);
vec3_t normalize_vec3(vec3_t a);
vec4_t normalize_vec4(vec4_t a);

vec2_t perp_vec2(vec2_t v);

vec2_t lerp_vec2(vec2_t a, vec2_t b, float t);
vec3_t lerp_vec3(vec3_t a, vec3_t b, float t);
vec4_t lerp_vec4(vec4_t a, vec4_t b, float t);

bool almost_eq_vec2(vec2_t a, vec2_t b, float eps);
bool almost_eq_vec3(vec3_t a, vec3_t b, float eps);
bool almost_eq_vec4(vec4_t a, vec4_t b, float eps);

#ifdef __cplusplus
}
#endif

#include "vec.hpp"

#endif // EK_MATH_VEC_H
