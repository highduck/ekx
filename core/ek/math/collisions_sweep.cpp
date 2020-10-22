#include "collisions.hpp"

namespace ek {

sweep_test_result_t sweep_rects(const rect_f& a0, const rect_f& a1, const rect_f& b0, const rect_f& b1) {
    sweep_test_result_t result;
    float dx = (a1.x - a0.x) - (b1.x - b0.x);
    float dy = (a1.y - a0.y) - (b1.y - b0.y);
    float cx0, cy0, cx1, cy1;
    float u0_x, u0_y, u1_x, u1_y;

    if (a0.overlaps(b0)) {
        result.u0 = 0.0f;
        result.u1 = 0.0f;
        result.normal = {
                a0.center_x() < b0.center_x() ? 1.0f : -1.0f,
                a0.center_y() < b0.center_y() ? 1.0f : -1.0f
        };
//			result.x = a0.centerX + 0.5 * (b0.centerX - a0.centerX);
//			result.y = a0.centerY + 0.5 * (b0.centerY - a0.centerY);
        result.hit = true;
        return result;
    }

    if (dx > 0.0f) {
        cx0 = b0.x - a0.right();
        cx1 = b0.right() - a0.x;
    } else {
        cx0 = b0.right() - a0.x;
        cx1 = b0.x - a0.right();
    }

    if (dy > 0.0f) {
        cy0 = b0.y - a0.bottom();
        cy1 = b0.bottom() - a0.y;
    } else {
        cy0 = b0.bottom() - a0.y;
        cy1 = b0.y - a0.bottom();
    }

    if (dx == 0.0f) {
        u0_x = 100000.0f;
        u1_x = -100000.0f;
    } else {
        u0_x = cx0 / dx;
        u1_x = cx1 / dx;
    }

    if (dy == 0.0f) {
        u0_y = 100000.0f;
        u1_y = -100000.0f;
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

    result.normal = float2::zero;

    if (u0_x == result.u0) {
        result.normal.x = dx > 0.0f ? 1.0f : -1.0f;
    }

    if (u0_y == result.u0) {
        result.normal.y = dy > 0.0f ? 1.0f : -1.0f;
    }

    return result;
}

sweep_test_result_t intersect_ray_rect(const rect_f& rc, const float2& origin, const float2& dir) {
    sweep_test_result_t result;
    result.u0 = 0.0f;
    result.u1 = 1000000.0f;
    result.hit = false;
    result.ray = false;
    if (fabsf(dir.x) < math::epsilon<float>()) {
        if (origin.x < rc.x || origin.x > rc.right()) {
            return result;
        }
    } else {
        float ood = 1.0f / dir.x;
        float t1 = ood * (rc.x - origin.x);
        float t2 = ood * (rc.right() - origin.x);
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

    if (fabsf(dir.y) < math::epsilon<float>()) {
        if (origin.y < rc.y || origin.y > rc.bottom()) {
            return result;
        }
    } else {
        float ood = 1.0f / dir.y;
        float t1 = ood * (rc.y - origin.y);
        float t2 = ood * (rc.bottom() - origin.y);

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
sweep_test_result_t sweep_circles(const circle_f& c0,
                                  const circle_f& c1,
                                  const float2& delta) {
    sweep_test_result_t result;
    const float2 s = c1.center - c0.center;
    const float r = c1.radius + c0.radius;
    const float c = dot(s, s) - r * r;
    if (c < 0.0f) {
        result.hit = true;
        result.u0 = 0.0f;
        return result;
    }
    const float a = dot(delta, delta);
    if (a <= math::epsilon<float>()) {
        return result;
    }
    const float b = dot(delta, s);
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

sweep_test_result_t sweep_circle_rect(const circle_f& circle,
                                      const float2& circle_delta,
                                      const rect_f& rect,
                                      const float2& rect_delta) {
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
    float2 origin = circle.center;
    float2 dir = rect_delta - circle_delta;
    float r = circle.radius;
//    float dx = rect_delta.x - circle_delta.x;
//    float dy = rect_delta.y - circle_delta.y;
    float2 cp = float2::zero;
    bool corner = false;

    float t0 = 1000000.0f;
    rect_f e{rect.x - r, rect.y - r, rect.width + r * 2.0f, rect.height + r * 2.0f};

    auto ii = intersect_ray_rect(e, origin, -dir);
    if (!ii.hit) {
        return ii;
    }

    e.set(rect.x - r, rect.y, rect.width + r * 2.0f, rect.height);
    ii = intersect_ray_rect(e, origin, -dir);
    if (ii.hit && ii.u0 <= 1.0f) {
        t0 = fmin(ii.u0, t0);
    }

    e.set(rect.x, rect.y - r, rect.width, rect.height + 2.0f * r);
    ii = intersect_ray_rect(e, origin, -dir);
    if (ii.hit && ii.u0 <= 1.0f) {
        t0 = fmin(ii.u0, t0);
    }

    circle_f origin_circle{origin, 0};
    ii = sweep_circles(origin_circle, {rect.x, rect.y, r}, dir);
    if (ii.hit && ii.u0 < t0) {
        t0 = ii.u0;
        cp = rect.position;
        corner = true;
    }

    ii = sweep_circles(origin_circle, {rect.right(), rect.y, r}, dir);
    if (ii.hit && ii.u0 < t0) {
        t0 = ii.u0;
        cp.x = rect.right();
        cp.y = rect.y;
        corner = true;
    }

    ii = sweep_circles(origin_circle, {rect.x, rect.bottom(), r}, dir);
    if (ii.hit && ii.u0 < t0) {
        t0 = ii.u0;
        cp.x = rect.x;
        cp.y = rect.bottom();
        corner = true;
    }

    ii = sweep_circles(origin_circle, {rect.right(), rect.bottom(), r}, dir);
    if (ii.hit && ii.u0 < t0) {
        t0 = ii.u0;
        cp.x = rect.right();
        cp.y = rect.bottom();
        corner = true;
    }

    ii.hit = t0 <= 1 && t0 >= 0;
    if (ii.hit) {
        origin += circle_delta * t0;
        float2 b = rect.position + rect_delta * t0;
        if (corner) {
            ii.normal = origin - cp - rect_delta * t0;
        } else if (origin.x >= b.x && origin.x <= b.x + rect.width) {
            ii.normal = {0.0f,
                         origin.y <= (b.y + rect.height * 0.5f) ? -1.0f : 1.0f};
        } else if (origin.y <= b.y + rect.height && origin.y >= b.y) {
            ii.normal = {origin.x <= (b.x + rect.width * 0.5) ? -1.0f : 1.0f,
                         0.0f};
        }
    }
    ii.u0 = t0;
    return ii;
}

float distance_to_rect(const rect_f& rc, const float2& p) {
    float s = 0.0f;
    float d = 0.0f;

    if (p.x < rc.x) {
        s = p.x - rc.x;
        d += s * s;
    } else if (p.x > rc.right()) {
        s = p.x - rc.right();
        d += s * s;
    }

    if (p.y < rc.y) {
        s = p.y - rc.y;
        d += s * s;
    } else if (p.y > rc.bottom()) {
        s = p.y - rc.bottom();
        d += s * s;
    }

    return sqrtf(d);
}

}