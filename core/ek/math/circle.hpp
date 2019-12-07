#pragma once

#include "vec.hpp"

namespace ek {

template<typename T>
struct circle_t {
    using vec_type = vec_t<T, 2>;

    vec_type center;
    T radius;

    inline explicit circle_t() noexcept
            : center{},
              radius{0} {

    }

    inline explicit circle_t(float radius_) noexcept
            : center{},
              radius{radius_} {

    }

    inline circle_t(const vec_type& center_, T radius_) noexcept
            : center{center_},
              radius{radius_} {

    }

    inline circle_t(T x, T y, T r = 0) noexcept
            : center{x, y},
              radius{r} {

    }
};

using circle_f = circle_t<float>;

}


