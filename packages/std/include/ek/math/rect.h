#ifndef EK_MATH_RECT_H
#define EK_MATH_RECT_H

#ifdef __cplusplus
extern "C" {
#endif


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

brect_t brect_from_rect(rect_t rc);
brect_t brect_inf(void);
brect_t brect_extend_circle(brect_t bb, vec3_t circle);
brect_t brect_extend_rect(brect_t bb, rect_t rc);
brect_t brect_extend_transformed_rect(brect_t bb, rect_t rc, mat3x2_t matrix);
brect_t brect_extend_point(brect_t bb, vec2_t point);
rect_t brect_get_rect(brect_t bb);

bool brect_is_empty(brect_t bb);

#ifdef __cplusplus
}
#endif

#include "rect.hpp"

#endif // EK_MATH_RECT_H
