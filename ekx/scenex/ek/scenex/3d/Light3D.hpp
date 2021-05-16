#pragma once

#include <ek/math/vec.hpp>

namespace ek {

enum class Light3DType {
    Directional,
    Point,
    Spot
};

struct Light3D {
    Light3DType type = Light3DType::Point;

    float3 ambient = 0.1f * float3::one;
    float3 diffuse = float3::one;
    float3 specular = float3::one;

    float radius = 100.0f;
    float falloff = 1.0f;

    bool cast_shadows = true;
};

EK_DECLARE_TYPE(Light3D);

}

