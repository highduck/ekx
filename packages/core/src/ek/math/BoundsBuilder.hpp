#pragma once

#include <cfloat>
#include "Vec.hpp"
#include "Rect.hpp"
#include "Matrix3x2.hpp"

namespace ek {

template<typename T>
class BoundsBuilder {
public:
    using vec_type = Vec2<T>;
    using rect_type = Rect2<T>;
    using mat3x2_type = Matrix<3, 2, T>;
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

    void add(const rect_type& rect, const mat3x2_type& matrix, T radius = T{0}) {
        add(matrix.transform(rect.x, rect.y), radius);
        add(matrix.transform(rect.right(), rect.y), radius);
        add(matrix.transform(rect.right(), rect.bottom()), radius);
        add(matrix.transform(rect.x, rect.bottom()), radius);
    }

    [[nodiscard]]
    bool empty() const {
        return (max.x - min.x) < Math::epsilon<T>() ||
               (max.y - min.y) < Math::epsilon<T>();
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

    static Rect2 <T> transform(const Rect2 <T>& rect, const Matrix3x2 <T>& matrix);
};

template<typename T>
Rect2 <T> BoundsBuilder<T>::transform(const Rect2 <T>& rect, const Matrix3x2 <T>& matrix) {
    BoundsBuilder bb;
    bb.add(rect, matrix);
    return bb.rect();
}

using BoundsBuilder2f = BoundsBuilder<float>;

}