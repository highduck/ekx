#pragma once

#include <ek/math.h>
#include <ek/serialize/serialize.hpp>

namespace ek {

struct ModelVertex3D {
    vec3_t position;
    vec3_t normal;
    vec2_t uv;
    rgba_t color;
    rgba_t color2;
};

template<>
struct declared_as_pod_type<ModelVertex3D> : public std::true_type {
};

struct Model3D {
    Array<ModelVertex3D> vertices;
    Array<uint16_t> indices;

    template<typename S>
    void serialize(IO<S>& io) {
        io(vertices, indices);
    }

    static Model3D createCube(vec3_t position, vec3_t size, argb32_t color = 0xFFFFFF_rgb);

    static Model3D createPlane(vec3_t position, vec2_t size, argb32_t color = 0xFFFFFF_rgb);
};


inline Model3D Model3D::createCube(const vec3_t position, const vec3_t size, argb32_t color) {
    Model3D result{};

    // 6 sides
    // 6 indices per size
    // 2 faces per size
    result.vertices.resize(6 * 4);
    result.indices.resize(6 * 2 * 3);

    vec3_t n;
    const float u = 0.5f;
    const auto color1 = color.abgr();
    const abgr32_t color2 = 0x0;

    // Top +Z
    n = vec3(0, 0, 1);
    result.vertices[0] = {vec3(-u, -u,u), n, vec2(0, 0), color1, color2};
    result.vertices[1] = {vec3(u, -u, u), n, vec2(0, 1), color1, color2};
    result.vertices[2] = {vec3(u, u,  u), n, vec2(1, 1), color1, color2};
    result.vertices[3] = {vec3(-u, u, u), n, vec2(1, 0), color1, color2};

    n = vec3(0, 0, -1);
    result.vertices[4] = {vec3(-u, -u,-u), n,vec2(0, 0), color1, color2};
    result.vertices[5] = {vec3(-u, u, -u), n,vec2(0, 1), color1, color2};
    result.vertices[6] = {vec3(u, u,  -u), n,vec2(1, 1), color1, color2};
    result.vertices[7] = {vec3(u, -u, -u), n,vec2(1, 0), color1, color2};

    n = vec3(1, 0, 0);
    result.vertices[8] =  {vec3(u, -u,  u), n, vec2(0, 0), color1, color2};
    result.vertices[9] =  {vec3(u, -u, -u), n, vec2(0, 1), color1, color2};
    result.vertices[10] = {vec3(u, u, -u), n, vec2(1, 1), color1, color2};
    result.vertices[11] = {vec3(u, u,  u), n, vec2(1, 0), color1, color2};

    n = vec3(-1, 0, 0);
    result.vertices[12] = {vec3(-u, u,   u), n, vec2(0, 0), color1, color2};
    result.vertices[13] = {vec3(-u, u,  -u), n, vec2(0, 1), color1, color2};
    result.vertices[14] = {vec3(-u, -u, -u), n, vec2(1, 1), color1, color2};
    result.vertices[15] = {vec3(-u, -u,  u), n, vec2(1, 0), color1, color2};

    n = vec3(0, -1, 0);
    result.vertices[16] = {vec3(-u, -u, u), n, vec2(0, 0), color1, color2};
    result.vertices[17] = {vec3(-u, -u, -u), n, vec2(0, 1), color1, color2};
    result.vertices[18] = {vec3(u, -u, -u), n, vec2(1, 1), color1, color2};
    result.vertices[19] = {vec3(u, -u, u), n, vec2(1, 0), color1, color2};

    n = vec3(0, 1, 0);
    result.vertices[20] = {vec3(u, u, u), n, vec2(0, 0), color1, color2};
    result.vertices[21] = {vec3(u, u, -u), n, vec2(0, 1), color1, color2};
    result.vertices[22] = {vec3(-u, u, -u), n, vec2(1, 1), color1, color2};
    result.vertices[23] = {vec3(-u, u, u), n, vec2(1, 0), color1, color2};

    // 12 * 3 = 36
    uint16_t indices[36]{
            0, 1, 2,
            2, 3, 0,
            4, 5, 6,
            6, 7, 4,
            8, 9, 10,
            10, 11, 8,
            12, 13, 14,
            14, 15, 12,
            16, 17, 18,
            18, 19, 16,
            20, 21, 22,
            22, 23, 20
    };
    memcpy(result.indices.data(), indices, sizeof(uint16_t) * 36);

    for (auto& v : result.vertices) {
        v.position.x = position.x + v.position.x * size.x;
        v.position.y = position.y + v.position.y * size.y;
        v.position.z = position.z + v.position.z * size.z;
    }

    return result;
}

inline Model3D Model3D::createPlane(const vec3_t position, const vec2_t size, argb32_t color) {
    Model3D result{};

    const float u = 0.5f;
    const auto color1 = color.abgr();
    const abgr32_t color2 = 0x0;
    const vec3_t n = vec3(0, 0, 1);

    result.vertices.resize(4);
    result.vertices[0] = {vec3(-u, -u, 0), n, vec2(0, 0), color1, color2};
    result.vertices[1] = {vec3(u, -u,  0), n, vec2(1, 0), color1, color2};
    result.vertices[2] = {vec3(u, u,   0), n, vec2(1, 1), color1, color2};
    result.vertices[3] = {vec3(-u, u,  0), n, vec2(0, 1), color1, color2};

    result.indices.resize(2 * 3);
    result.indices[0] = 0;
    result.indices[1] = 1;
    result.indices[2] = 2;
    result.indices[3] = 2;
    result.indices[4] = 3;
    result.indices[5] = 0;

    for (auto& v : result.vertices) {
        v.position.x = position.x + v.position.x * size.x;
        v.position.y = position.y + v.position.y * size.y;
        v.position.z = position.z + v.position.z;
    }

    return result;
}
}

