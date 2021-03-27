#pragma once

#include <ek/math/vec.hpp>

namespace ek {

enum class light_3d_type {
    directional,
    point,
    spot
};

struct Light3D {
    light_3d_type type = light_3d_type::point;

    float3 ambient = 0.1f * float3::one;
    float3 diffuse = float3::one;
    float3 specular = float3::one;

    float radius = 100.0f;
    float falloff = 1.0f;

    bool cast_shadows = true;
};

}

