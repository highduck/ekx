#include "collisions.hpp"

namespace ek {

bool test_point_triangle(const float2& point,
                         const float2& v0,
                         const float2& v1,
                         const float2& v2) {
    const auto m = sign(point, v1, v2) < 0.0f;
    return m == (sign(point, v0, v1) < 0.0f) && m == (sign(point, v2, v0) < 0.0f);
}

bool test_line_line(const float2& a,
                    const float2& b,
                    const float2& c,
                    const float2& d,
                    bool segment_mode,
                    float2& intersection) {
    auto a1 = b.y - a.y;
    auto a2 = d.y - c.y;
    auto b1 = a.x - b.x;
    auto b2 = c.x - d.x;
    auto c1 = b.x * a.y - a.x * b.y;
    auto c2 = d.x * c.y - c.x * d.y;

    auto denom = a1 * b2 - a2 * b1;
    if (denom == 0.0f) {
        return false;
    }

    float2 ip{(b1 * c2 - b2 * c1) / denom, (a2 * c1 - a1 * c2) / denom};

    if (segment_mode) {
        float len = distance_sqr(a, b);
        if (distance_sqr(ip, b) > len || distance_sqr(ip, a) > len) {
            return false;
        }

        len = distance_sqr(c, d);
        if (distance_sqr(ip, d) > len || distance_sqr(ip, c) > len) {
            return false;
        }
    }

    intersection = ip;
    return true;
}

bool test_rect_line(const rect_f& rect, const float2& p0, const float2& p1) {

// Calculate m and c for the equation for the line (y = mx+c)
    auto m = (p1.y - p0.y) / (p1.x - p0.x);
    auto c = p0.y - (m * p0.x);

    auto l = rect.left();
    auto r = rect.right();

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
    return top_overlap < bottom_overlap
           && bottom_overlap >= rect.y
           && top_overlap <= rect.bottom();
}


bool test_rect_triangle(const rect_f& rect,
                        const float2& v0,
                        const float2& v1,
                        const float2& v2) {
    return test_rect_line(rect, v0, v1)
           || test_rect_line(rect, v1, v2)
           || test_rect_line(rect, v2, v0);
}

}