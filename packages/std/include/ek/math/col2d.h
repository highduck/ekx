#ifndef EK_MATH_COL2D_H
#define EK_MATH_COL2D_H

#include "vec.h"
#include "rect.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    // future collision
    SWEEP_TEST_RAY = 1,
    // frame collision
    SWEEP_TEST_HIT = 2
};

typedef struct sweep_test_t {
    // contact normal
    vec2_t normal;

    // contact position
    vec2_t contact;

    //normalized time of first collision
    float u0;

    //normalized time of second collision
    float u1;

    unsigned flags;
} sweep_test_t;

float distance_to_rect(rect_t rc, vec2_t p);

sweep_test_t intersect_ray_rect(rect_t rc, vec2_t origin, vec2_t dir);

sweep_test_t sweep_circles(vec3_t c0, vec3_t c1, vec2_t delta);

sweep_test_t sweep_rects(rect_t a0, rect_t a1, rect_t b0, rect_t b1);

sweep_test_t sweep_circle_rect(vec3_t circle, vec2_t circle_delta, rect_t rect, vec2_t rect_delta);

/*** Tests ***/
bool test_rect_triangle(rect_t rect, vec2_t v0, vec2_t v1, vec2_t v2);

bool test_rect_line(rect_t rect, vec2_t p0, vec2_t p1);

bool test_line_line(vec2_t a, vec2_t b, vec2_t c, vec2_t d, bool segment_mode, vec2_t* intersection);

bool test_point_triangle(vec2_t point, vec2_t v0, vec2_t v1, vec2_t v2);

#ifdef __cplusplus
}
#endif

#endif // EK_MATH_COL2D_H
