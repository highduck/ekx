#pragma once

#include <ecxx/ecxx_fwd.hpp>
#include <ek/math/Matrix4.hpp>
#include <ek/math/Vec.hpp>
#include <ek/util/Type.hpp>

namespace ecs {
class World;
}

namespace ek {

struct Transform3D {
    Matrix4f local{};
    Matrix4f world{};
    Vec3f position{};
    Vec3f scale = Vec3f::one;
    Vec3f rotation{};

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

