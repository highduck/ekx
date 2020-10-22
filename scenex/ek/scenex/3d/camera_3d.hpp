#pragma once

#include <ek/math/common.hpp>
#include <string>
#include <ek/math/vec.hpp>

#undef near
#undef far

namespace ek {

struct camera_3d {
    // clip plane near-far
    float near = 10.0f;
    float far = 1000.0f;

    // orthogonal mode
    bool orthogonal = false;
    float orthogonal_size = 30.0f;

    // field of view in radians
    float fov = math::to_radians(45.0f);

    // camera up vector
    float3 up{0.0f, 0.0f, 1.0f};

    bool clear_color_enabled = true;
    float4 clear_color{0.5f, 0.5f, 0.5f, 1.0f};
    bool clear_depth_enabled = true;
    float clear_depth = 1.0f;

    std::string cube_map;
};

}

