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
#ifdef __cplusplus

    inline float& operator[](const int index) { return data[index]; }

#endif
} vec3_t;

typedef union vec4_t {
    struct {
        float x;
        float y;
        float z;
        float w;
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
vec4_t vec4_v(vec3_t v, float w);

vec2_t vec2_max(vec2_t a, vec2_t b);
vec3_t vec3_max(vec3_t a, vec3_t b);
vec4_t vec4_max(vec4_t a, vec4_t b);

vec2_t vec2_min(vec2_t a, vec2_t b);
vec3_t vec3_min(vec3_t a, vec3_t b);
vec4_t vec4_min(vec4_t a, vec4_t b);

float vec2_dot(vec2_t a, vec2_t b);
float vec3_dot(vec3_t a, vec3_t b);
float vec4_dot(vec4_t a, vec4_t b);

float vec2_distance_sqr(vec2_t a, vec2_t b);
float vec3_distance_sqr(vec3_t a, vec3_t b);
float vec4_distance_sqr(vec4_t a, vec4_t b);

float vec2_distance(vec2_t a, vec2_t b);
float vec3_distance(vec3_t a, vec3_t b);
float vec4_distance(vec4_t a, vec4_t b);

float vec2_length_sqr(vec2_t a);
float vec3_length_sqr(vec3_t a);
float vec4_length_sqr(vec4_t a);

float vec2_length(vec2_t a);
float vec3_length(vec3_t a);
float vec4_length(vec4_t a);

vec2_t vec2_add(vec2_t a, vec2_t b);
vec3_t vec3_add(vec3_t a, vec3_t b);
vec4_t vec4_add(vec4_t a, vec4_t b);

vec2_t vec2_sub(vec2_t a, vec2_t b);
vec3_t vec3_sub(vec3_t a, vec3_t b);
vec4_t vec4_sub(vec4_t a, vec4_t b);

vec2_t vec2_neg(vec2_t a);
vec3_t vec3_neg(vec3_t a);
vec4_t vec4_neg(vec4_t a);

vec3_t vec3_cross(vec3_t a, vec3_t b);

vec2_t vec2_scale(vec2_t a, float s);
vec3_t vec3_scale(vec3_t a, float s);
vec4_t vec4_scale(vec4_t a, float s);

vec2_t vec2_mul(vec2_t a, vec2_t b);
vec3_t vec3_mul(vec3_t a, vec3_t b);
vec4_t vec4_mul(vec4_t a, vec4_t b);

vec2_t vec2_normalize(vec2_t a);
vec3_t vec3_normalize(vec3_t a);
vec4_t vec4_normalize(vec4_t a);

vec2_t vec2_perp(vec2_t v);

vec2_t vec2_lerp(vec2_t a, vec2_t b, float t);
vec3_t vec3_lerp(vec3_t a, vec3_t b, float t);
vec4_t vec4_lerp(vec4_t a, vec4_t b, float t);

bool almost_eq_vec2(vec2_t a, vec2_t b, float eps);
bool almost_eq_vec3(vec3_t a, vec3_t b, float eps);
bool almost_eq_vec4(vec4_t a, vec4_t b, float eps);

#ifdef __cplusplus
}
#endif

#include "vec.hpp"

#endif // EK_MATH_VEC_H
