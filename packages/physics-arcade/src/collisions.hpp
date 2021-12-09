#pragma once

#include <ek/math/Vec.hpp>
#include <ek/math/Rect.hpp>
#include <ek/math/Circle.hpp>

namespace ek {

struct sweep_test_result_t {

    // future collision
    bool ray = false;

    // frame collision
    bool hit = false;

    // contact normal
    Vec2f normal{};

    // contact position
    Vec2f contact{};

    //normalized time of first collision
    float u0 = 0.0f;

    //normalized time of second collision
    float u1 = 0.0f;
};

float distance_to_rect(const Rect2f& rc, const Vec2f& p);

sweep_test_result_t intersect_ray_rect(const Rect2f& rc, const Vec2f& origin, const Vec2f& dir);

sweep_test_result_t sweep_circles(const CircleF& c0,
                                  const CircleF& c1,
                                  const Vec2f& delta);

sweep_test_result_t sweep_rects(const Rect2f& a0, const Rect2f& a1, const Rect2f& b0, const Rect2f& b1);

sweep_test_result_t sweep_circle_rect(const CircleF& circle, const Vec2f& circle_delta,
                                      const Rect2f& rect, const Vec2f& rect_delta);


/*** Tests ***/
bool test_rect_triangle(const Rect2f& rect,
                        const Vec2f& v0,
                        const Vec2f& v1,
                        const Vec2f& v2);

bool test_rect_line(const Rect2f& rect, const Vec2f& p0, const Vec2f& p1);

bool test_line_line(const Vec2f& a,
                    const Vec2f& b,
                    const Vec2f& c,
                    const Vec2f& d,
                    bool segment_mode,
                    Vec2f& intersection);

bool test_point_triangle(const Vec2f& point,
                         const Vec2f& v0,
                         const Vec2f& v1,
                         const Vec2f& v2);
}


