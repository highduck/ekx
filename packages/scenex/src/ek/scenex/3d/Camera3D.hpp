#pragma once

#include <ek/math.h>
#include <ek/util/Res.hpp>
#include <ek/util/Type.hpp>
#include <ecxx/ecxx_fwd.hpp>
#include <ek/gfx.h>

namespace ek {

struct Camera3D {
    // clip plane near-far
    float zNear = 10.0f;
    float zFar = 1000.0f;

    // orthogonal mode
    bool orthogonal = false;
    float orthogonalSize = 30.0f;

    // field of view in radians
    float fov = to_radians(45.0f);

    // camera up vector
    vec3_t up = vec3(0, 0,1);

    bool clearColorEnabled = true;
    bool clearDepthEnabled = true;

    vec4_t clearColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    float clearDepth = 1.0f;

    ek_ref(sg_image) cubeMap;
};

ECX_TYPE(29, Camera3D);

}

