#pragma once

#include <ek/math/common.hpp>
#include <ek/math/vec.hpp>
#include <string>
#include <ek/util/assets.hpp>

#undef near
#undef far

namespace ek {

namespace graphics {
class texture_t;
}

struct Camera3D {
    // clip plane near-far
    float near = 10.0f;
    float far = 1000.0f;

    // orthogonal mode
    bool orthogonal = false;
    float orthogonalSize = 30.0f;

    // field of view in radians
    float fov = math::to_radians(45.0f);

    // camera up vector
    float3 up{0.0f, 0.0f, 1.0f};

    bool clearColorEnabled = true;
    bool clearDepthEnabled = true;

    float4 clearColor{0.5f, 0.5f, 0.5f, 1.0f};
    float clearDepth = 1.0f;

    Res<graphics::texture_t> cubeMap;
};

}

