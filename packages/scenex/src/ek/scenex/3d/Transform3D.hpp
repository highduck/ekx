#pragma once

#include <ecxx/ecxx_fwd.hpp>
#include <ek/math.h>
#include <ek/util/Type.hpp>

namespace ecs {
class World;
}

namespace ek {

struct Transform3D {
    mat4_t local = mat4_identity();
    mat4_t world = mat4_identity();
    vec3_t position = {};
    vec3_t scale = vec3(1,1,1);
    vec3_t rotation = {};

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

ECX_TYPE(27, Transform3D);

}

