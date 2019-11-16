#pragma once

#include "vec.hpp"
#include "box.hpp"
#include "circle.hpp"

namespace ek {

struct sweep_test_result_t {

    // future collision
    bool ray = false;

    // frame collision
    bool hit = false;

    // contact normal
    float2 normal{};

    // contact position
    float2 contact{};

    //normalized time of first collision
    float u0 = 0.0f;

    //normalized time of second collision
    float u1 = 0.0f;
};

float distance_to_rect(const rect_f& rc, const float2& p);

sweep_test_result_t intersect_ray_rect(const rect_f& rc, const float2& origin, const float2& dir);

sweep_test_result_t sweep_circles(const circle_f& c0,
                                  const circle_f& c1,
                                  const float2& delta);

sweep_test_result_t sweep_rects(const rect_f& a0, const rect_f& a1, const rect_f& b0, const rect_f& b1);

sweep_test_result_t sweep_circle_rect(const circle_f& circle, const float2& circle_delta,
                                      const rect_f& rect, const float2& rect_delta);


/*** Tests ***/
bool test_rect_triangle(const rect_f& rect,
                        const float2& v0,
                        const float2& v1,
                        const float2& v2);

bool test_rect_line(const rect_f& rect, const float2& p0, const float2& p1);

bool test_line_line(const float2& a,
                    const float2& b,
                    const float2& c,
                    const float2& d,
                    bool segment_mode,
                    float2& intersection);

bool test_point_triangle(const float2& point,
                         const float2& v0,
                         const float2& v1,
                         const float2& v2);
}


