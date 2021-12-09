#pragma once

#include <ek/math/Vec.hpp>

namespace ek {

enum class Light3DType {
    Directional,
    Point,
    Spot
};

struct Light3D {
    Light3DType type = Light3DType::Point;

    Vec3f ambient = 0.1f * Vec3f::one;
    Vec3f diffuse = Vec3f::one;
    Vec3f specular = Vec3f::one;

    float radius = 100.0f;
    float falloff = 1.0f;

    bool cast_shadows = true;
};

ECX_TYPE(28, Light3D);

}

