#pragma once

#include "Vec.hpp"

namespace ek {

template<typename T>
struct Circle {
    using vec_type = Vec<2, T>;

    vec_type center;
    T radius;

    constexpr explicit Circle() noexcept: center{},
                                          radius{0} {

    }

    constexpr explicit Circle(float radius_) noexcept: center{},
                                                       radius{radius_} {

    }

    constexpr Circle(const vec_type& center_, T radius_) noexcept: center{center_},
                                                                   radius{radius_} {

    }

    constexpr Circle(T x, T y, T r = 0) noexcept: center{x, y},
                                                  radius{r} {

    }
};

using CircleF = Circle<float>;

}


