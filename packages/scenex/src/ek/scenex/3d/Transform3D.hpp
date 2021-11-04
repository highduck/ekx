#pragma once

#include <ek/math/mat4x4.hpp>
#include <ek/math/vec.hpp>
#include <ek/util/Type.hpp>

namespace ek {

struct Transform3D {
    mat4f local{};
    mat4f world{};
    float3 position{};
    float3 scale = float3::one;
    float3 rotation{};

//    inline void set_euler_angles(const float3& angles) {
//        mat4f m = euler_angles(angles);
////        rotation = quat_t<float>{euler_angles};
//        rotation = quat_t<float>{m};
//    }
//
//    inline float3 get_euler_angles() const {
//        euler_angles(rotation);
//    }

    static void updateAll();
};

EK_DECLARE_TYPE(Transform3D);

}

