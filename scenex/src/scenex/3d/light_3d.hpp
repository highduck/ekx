#pragma once

#include <ek/math/vec.hpp>

namespace scenex {

enum class light_3d_type {
    directional,
    point,
    spot
};

struct light_3d {
    light_3d_type type = light_3d_type::point;

    ek::float3 ambient = 0.1f * ek::float3::one;
    ek::float3 diffuse = ek::float3::one;
    ek::float3 specular = ek::float3::one;

    float radius = 100.0f;
    float falloff = 1.0f;

    bool cast_shadows = true;
};

}

