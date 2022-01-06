#pragma once

#include "collisions.hpp"

namespace ek {

/** Collision test functions **/
inline float vec2_sign(const vec2_t p1, const vec2_t p2, const vec2_t p3) {
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool test_point_triangle(const vec2_t point, const vec2_t v0, const vec2_t v1, const vec2_t v2) {
    const bool m = vec2_sign(point, v1, v2) < 0.0f;
    return m == (vec2_sign(point, v0, v1) < 0.0f) && m == (vec2_sign(point, v2, v0) < 0.0f);
}

bool test_line_line(const vec2_t a, const vec2_t b, const vec2_t c, const vec2_t d, bool segment_mode,
                    vec2_t* intersection) {
    const float a1 = b.y - a.y;
    const float a2 = d.y - c.y;
    const float b1 = a.x - b.x;
    const float b2 = c.x - d.x;
    const float c1 = b.x * a.y - a.x * b.y;
    const float c2 = d.x * c.y - c.x * d.y;

    const float denom = a1 * b2 - a2 * b1;
    if (denom == 0.0f) {
        return false;
    }

    const vec2_t ip{(b1 * c2 - b2 * c1) / denom, (a2 * c1 - a1 * c2) / denom};

    if (segment_mode) {
        float len = vec2_distance_sqr(a, b);
        if (vec2_distance_sqr(ip, b) > len || vec2_distance_sqr(ip, a) > len) {
            return false;
        }

        len = vec2_distance_sqr(c, d);
        if (vec2_distance_sqr(ip, d) > len || vec2_distance_sqr(ip, c) > len) {
            return false;
        }
    }

    if (intersection) {
        *intersection = ip;
    }
    return true;
}

bool test_rect_line(const rect_t rect, const vec2_t p0, const vec2_t p1) {

// Calculate m and c for the equation for the line (y = mx+c)
    auto m = (p1.y - p0.y) / (p1.x - p0.x);
    auto c = p0.y - (m * p0.x);

    auto l = rect.x;
    auto r = RECT_R(rect);

    float top_intersection;
    float bottom_intersection;

// if the line is going up from right to left then the top intersect point is on the left
    if (m > 0.0f) {
        top_intersection = m * l + c;
        bottom_intersection = m * r + c;
    } else {
// otherwise it's on the right
        top_intersection = m * r + c;
        bottom_intersection = m * l + c;
    }

// work out the top and bottom extents for the triangle
    float top_triangle_point = p0.y < p1.y ? p0.y : p1.y;
    float bottom_triangle_point = p0.y < p1.y ? p1.y : p0.y;

// and calculate the overlap between those two bounds
    float top_overlap = top_intersection > top_triangle_point ? top_intersection : top_triangle_point;
    float bottom_overlap = bottom_intersection < bottom_triangle_point ? bottom_intersection : bottom_triangle_point;

// (topoverlap<botoverlap) :
// if the intersection isn't the right way up then we have no overlap

// (!((botoverlap<t) || (topoverlap>b)) :
// If the bottom overlap is higher than the top of the rectangle or the top overlap is
// lower than the bottom of the rectangle we don't have intersection. So return the negative
// of that. Much faster than checking each of the points is within the bounds of the rectangle.
    return top_overlap < bottom_overlap &&
           bottom_overlap >= rect.y &&
           top_overlap <= RECT_B(rect);
}


bool test_rect_triangle(const rect_t rect,
                        const vec2_t v0,
                        const vec2_t v1,
                        const vec2_t v2) {
    return test_rect_line(rect, v0, v1)
           || test_rect_line(rect, v1, v2)
           || test_rect_line(rect, v2, v0);
}


/** Collision sweep functions **/

sweep_test_result_t sweep_rects(const rect_t a0, const rect_t a1, const rect_t b0, const rect_t b1) {
    sweep_test_result_t result;
    float dx = (a1.x - a0.x) - (b1.x - b0.x);
    float dy = (a1.y - a0.y) - (b1.y - b0.y);
    float cx0, cy0, cx1, cy1;
    float u0_x, u0_y, u1_x, u1_y;

    if (rect_overlaps(a0, b0)) {
        result.u0 = 0.0f;
        result.u1 = 0.0f;
        result.normal.x = RECT_CENTER_X(a0) < RECT_CENTER_X(b0) ? 1.0f : -1.0f;
        result.normal.y = RECT_CENTER_Y(a0) < RECT_CENTER_Y(b0) ? 1.0f : -1.0f;
        // result.x = a0.centerX + 0.5 * (b0.centerX - a0.centerX);
        // result.y = a0.centerY + 0.5 * (b0.centerY - a0.centerY);
        result.hit = true;
        return result;
    }

    if (dx > 0.0f) {
        cx0 = b0.x - RECT_R(a0);
        cx1 = RECT_R(b0) - a0.x;
    } else {
        cx0 = RECT_R(b0) - a0.x;
        cx1 = b0.x - RECT_R(a0);
    }

    if (dy > 0.0f) {
        cy0 = b0.y - RECT_B(a0);
        cy1 = RECT_B(b0) - a0.y;
    } else {
        cy0 = RECT_B(b0) - a0.y;
        cy1 = b0.y - RECT_B(a0);
    }

    if (dx == 0.0f) {
        u0_x = -100000.0f;
        u1_x = 100000.0f;
    } else {
        u0_x = cx0 / dx;
        u1_x = cx1 / dx;
    }

    if (dy == 0.0f) {
        u0_y = -100000.0f;
        u1_y = 100000.0f;
    } else {
        u0_y = cy0 / dy;
        u1_y = cy1 / dy;
    }

    result.u0 = fmax(u0_x, u0_y);
    result.u1 = fmin(u1_x, u1_y);
    result.ray = result.u0 <= result.u1;
    result.hit = result.ray
                 && (result.u0 <= 1.0f)
                 && (result.u0 >= 0.0f);

    result.normal = vec2(0.0f, 0.0f);

    if (u0_x == result.u0) {
        result.normal.x = dx > 0.0f ? 1.0f : -1.0f;
    }

    if (u0_y == result.u0) {
        result.normal.y = dy > 0.0f ? 1.0f : -1.0f;
    }

    return result;
}

sweep_test_result_t intersect_ray_rect(const rect_t rc, const vec2_t origin, const vec2_t dir) {
    sweep_test_result_t result;
    result.u0 = 0.0f;
    result.u1 = 1000000.0f;
    result.hit = false;
    result.ray = false;
    if (fabsf(dir.x) < MATH_F32_EPSILON) {
        if ((origin.x < rc.x) || (origin.x > RECT_R(rc))) {
            return result;
        }
    } else {
        float ood = 1.0f / dir.x;
        float t1 = ood * (rc.x - origin.x);
        float t2 = ood * (RECT_R(rc) - origin.x);
        if (t1 > t2) {
            float tmp = t1;
            t1 = t2;
            t2 = tmp;
        }
        result.u0 = fmax(result.u0, t1);
        result.u1 = fmin(result.u1, t2);
        if (result.u0 > result.u1) {
            // fail
            return result;
        }
    }

    if (fabsf(dir.y) < MATH_F32_EPSILON) {
        if ((origin.y < rc.y) || (origin.y > RECT_B(rc))) {
            return result;
        }
    } else {
        float ood = 1.0f / dir.y;
        float t1 = ood * (rc.y - origin.y);
        float t2 = ood * (RECT_B(rc) - origin.y);

        if (t1 > t2) {
            float tmp = t1;
            t1 = t2;
            t2 = tmp;
        }
        result.u0 = fmax(result.u0, t1);
        result.u1 = fmin(result.u1, t2);
        if (result.u0 > result.u1) {
// fail
            return result;
        }
    }
    result.ray = true;
    result.hit = true;
    result.contact = origin + dir * result.u0;
    return result;
}

// d = v1 - v0
sweep_test_result_t sweep_circles(const circle_t c0,
                                  const circle_t c1,
                                  const vec2_t delta) {
    sweep_test_result_t result;
    const vec2_t s = c1.center - c0.center;
    const float r = c1.radius + c0.radius;
    const float c = vec2_dot(s, s) - r * r;
    if (c < 0.0f) {
        result.hit = true;
        result.u0 = 0.0f;
        return result;
    }
    const float a = vec2_dot(delta, delta);
    if (a <= MATH_F32_EPSILON) {
        return result;
    }
    const float b = vec2_dot(delta, s);
    if (b >= 0) {
        return result;
    }
    const float d = b * b - a * c;
    if (d < 0) {
        return result;
    }
    result.u0 = (-b - sqrtf(d)) / a;
    result.hit = true;//result.u0;
    return result;
}

// d = box_v - c_v

sweep_test_result_t sweep_circle_rect(const circle_t circle,
                                      const vec2_t circle_delta,
                                      const rect_t rect,
                                      const vec2_t rect_delta) {
// TODO: optimize allocation
//		var e = new rect_f(b.x - r, b.y - r, b.width + r * 2, b.height + r * 2);
//		var ii = intersectRayRect(e, x, y, dx, dy);
//		if(!ii.hit) {
//			return ii;
//		}
//		var px = x + dx * ii.u0;
//		var py = y + dy * ii.u0;
//		var u = 0;
//		var v = 0;
//		if(px < b.x) u |= 1;
//		if(px > b.right) v |= 1;
//		if(py < b.y) u |= 2;
//		if(py > b.bottom) v |= 2;
//
//		var m = u + v;

//    float x = circle.center.x;
//    float y = circle.center.y;
    vec2_t origin = circle.center;
    vec2_t dir = rect_delta - circle_delta;
    float r = circle.radius;
//    float dx = rect_delta.x - circle_delta.x;
//    float dy = rect_delta.y - circle_delta.y;
    vec2_t cp = {};
    bool corner = false;

    float t0 = 1000000.0f;
    rect_t e{{rect.x - r, rect.y - r, rect.w + r * 2.0f, rect.h + r * 2.0f}};

    auto ii = intersect_ray_rect(e, origin, -dir);
    if (!ii.hit) {
        return ii;
    }

    e = {{rect.x - r, rect.y, rect.w + r * 2.0f, rect.h}};
    ii = intersect_ray_rect(e, origin, -dir);
    if (ii.hit && ii.u0 <= 1.0f) {
        t0 = fmin(ii.u0, t0);
    }

    e = {{rect.x, rect.y - r, rect.w, rect.h + 2.0f * r}};
    ii = intersect_ray_rect(e, origin, -dir);
    if (ii.hit && ii.u0 <= 1.0f) {
        t0 = fmin(ii.u0, t0);
    }

    circle_t origin_circle{origin, 0};
    ii = sweep_circles(origin_circle, {rect.x, rect.y, r}, dir);
    if (ii.hit && ii.u0 < t0) {
        t0 = ii.u0;
        cp = rect.position;
        corner = true;
    }

    ii = sweep_circles(origin_circle, {RECT_R(rect), rect.y, r}, dir);
    if (ii.hit && ii.u0 < t0) {
        t0 = ii.u0;
        cp.x = RECT_R(rect);
        cp.y = rect.y;
        corner = true;
    }

    ii = sweep_circles(origin_circle, {rect.x, RECT_B(rect), r}, dir);
    if (ii.hit && ii.u0 < t0) {
        t0 = ii.u0;
        cp.x = rect.x;
        cp.y = RECT_B(rect);
        corner = true;
    }

    ii = sweep_circles(origin_circle, {rect_rb(rect), r}, dir);
    if (ii.hit && ii.u0 < t0) {
        t0 = ii.u0;
        cp = rect_rb(rect);
        corner = true;
    }

    ii.hit = t0 <= 1 && t0 >= 0;
    if (ii.hit) {
        origin += circle_delta * t0;
        vec2_t b = rect.position + rect_delta * t0;
        if (corner) {
            ii.normal = origin - cp - rect_delta * t0;
        } else if (origin.x >= b.x && origin.x <= b.x + rect.w) {
            ii.normal = {0.0f, origin.y <= (b.y + rect.h * 0.5f) ? -1.0f : 1.0f};
        } else if (origin.y <= b.y + rect.h && origin.y >= b.y) {
            ii.normal = {origin.x <= (b.x + rect.w * 0.5f) ? -1.0f : 1.0f, 0.0f};
        }
    }
    ii.u0 = t0;
    return ii;
}

float distance_to_rect(const rect_t rc, const vec2_t p) {
    float s = 0.0f;
    float d = 0.0f;

    if (p.x < rc.x) {
        s = p.x - rc.x;
        d += s * s;
    } else if (p.x > RECT_R(rc)) {
        s = p.x - RECT_R(rc);
        d += s * s;
    }

    if (p.y < rc.y) {
        s = p.y - rc.y;
        d += s * s;
    } else if (p.y > RECT_B(rc)) {
        s = p.y - RECT_B(rc);
        d += s * s;
    }

    return sqrtf(d);
}

}