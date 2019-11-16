#pragma once

#include <cfloat>
#include "vec.hpp"
#include "box.hpp"

namespace ek {

template<typename T>
class bounds_builder_t {
public:
    using vec_type = vec2_t<T>;
    using rect_type = rect_t<T>;
    constexpr static T limit_value = FLT_MAX;

    vec_type min{limit_value, limit_value};
    vec_type max{-limit_value, -limit_value};

    void add(const vec_type& center, T radius = T{0}) {
        if (center.x - radius < min.x) min.x = center.x - radius;
        if (center.y - radius < min.y) min.y = center.y - radius;
        if (center.x + radius > max.x) max.x = center.x + radius;
        if (center.y + radius > max.y) max.y = center.y + radius;
    }

    void add(const vec_type& left_top, const vec_type& right_bottom) {
        if (left_top.x < min.x) min.x = left_top.x;
        if (right_bottom.x > max.x) max.x = right_bottom.x;
        if (left_top.y < min.y) min.y = left_top.y;
        if (right_bottom.y > max.y) max.y = right_bottom.y;
    }

    void add(const rect_type& rect) {
        add(rect.position, rect.position + rect.size);
    }

    bool empty() const {
        return (max.x - min.x) < math::epsilon<T>() ||
               (max.y - min.y) < math::epsilon<T>();
    }

    inline rect_type rect() const {
        return {min, size()};
    }

    inline vec_type size() const {
        return {width(), height()};
    }

    inline T width() const {
        return max.x > min.x ? (max.x - min.x) : 0.0f;
    }

    inline T height() const {
        return max.y > min.y ? (max.y - min.y) : 0.0f;
    }
};

using bounds_builder_2f = bounds_builder_t<float>;

}
