#pragma once

#include <ek/math/Color32.hpp>
#include <ek/math/Vec.hpp>
#include <ek/serialize/serialize.hpp>

namespace ek {

struct ModelVertex3D {
    Vec3f position;
    Vec3f normal;
    Vec2f uv;
    abgr32_t color;
    abgr32_t color2;
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

    static Model3D createCube(const Vec3f& position, const Vec3f& size, argb32_t color = 0xFFFFFF_rgb);

    static Model3D createPlane(const Vec3f& position, const Vec2f& size, argb32_t color = 0xFFFFFF_rgb);
};


inline Model3D Model3D::createCube(const Vec3f& position, const Vec3f& size, argb32_t color) {
    Model3D result{};

    // 6 sides
    // 6 indices per size
    // 2 faces per size
    result.vertices.resize(6 * 4);
    result.indices.resize(6 * 2 * 3);

    Vec3f n;
    const float u = 0.5f;
    const auto color1 = color.abgr();
    const abgr32_t color2 = 0x0;

    // Top +Z
    n = {0, 0, 1};
    result.vertices[0] = {{-u, -u, u}, n, {0, 0}, color1, color2};
    result.vertices[1] = {{u, -u, u}, n, {0, 1}, color1, color2};
    result.vertices[2] = {{u, u, u}, n, {1, 1}, color1, color2};
    result.vertices[3] = {{-u, u, u}, n, {1, 0}, color1, color2};

    n = {0, 0, -1};
    result.vertices[4] = {{-u, -u, -u}, n, {0, 0}, color1, color2};
    result.vertices[5] = {{-u, u, -u}, n, {0, 1}, color1, color2};
    result.vertices[6] = {{u, u, -u}, n, {1, 1}, color1, color2};
    result.vertices[7] = {{u, -u, -u}, n, {1, 0}, color1, color2};

    n = {1, 0, 0};
    result.vertices[8] = {{u, -u, u}, n, {0, 0}, color1, color2};
    result.vertices[9] = {{u, -u, -u}, n, {0, 1}, color1, color2};
    result.vertices[10] = {{u, u, -u}, n, {1, 1}, color1, color2};
    result.vertices[11] = {{u, u, u}, n, {1, 0}, color1, color2};

    n = {-1, 0, 0};
    result.vertices[12] = {{-u, u, u}, n, {0, 0}, color1, color2};
    result.vertices[13] = {{-u, u, -u}, n, {0, 1}, color1, color2};
    result.vertices[14] = {{-u, -u, -u}, n, {1, 1}, color1, color2};
    result.vertices[15] = {{-u, -u, u}, n, {1, 0}, color1, color2};

    n = {0, -1, 0};
    result.vertices[16] = {{-u, -u, u}, n, {0, 0}, color1, color2};
    result.vertices[17] = {{-u, -u, -u}, n, {0, 1}, color1, color2};
    result.vertices[18] = {{u, -u, -u}, n, {1, 1}, color1, color2};
    result.vertices[19] = {{u, -u, u}, n, {1, 0}, color1, color2};

    n = {0, 1, 0};
    result.vertices[20] = {{u, u, u}, n, {0, 0}, color1, color2};
    result.vertices[21] = {{u, u, -u}, n, {0, 1}, color1, color2};
    result.vertices[22] = {{-u, u, -u}, n, {1, 1}, color1, color2};
    result.vertices[23] = {{-u, u, u}, n, {1, 0}, color1, color2};

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
        v.position = position + v.position * size;
    }

    return result;
}

inline Model3D Model3D::createPlane(const Vec3f& position, const Vec2f& size, argb32_t color) {
    Model3D result{};

    const float u = 0.5f;
    const auto color1 = color.abgr();
    const abgr32_t color2 = 0x0;
    const Vec3f n{0, 0, 1};

    result.vertices.resize(4);
    result.vertices[0] = {{-u, -u, 0.0f}, n, {0, 0}, color1, color2};
    result.vertices[1] = {{u, -u, 0.0f}, n, {1, 0}, color1, color2};
    result.vertices[2] = {{u, u, 0.0f}, n, {1, 1}, color1, color2};
    result.vertices[3] = {{-u, u, 0.0f}, n, {0, 1}, color1, color2};

    result.indices.resize(2 * 3);
    result.indices[0] = 0;
    result.indices[1] = 1;
    result.indices[2] = 2;
    result.indices[3] = 2;
    result.indices[4] = 3;
    result.indices[5] = 0;

    for (auto& v : result.vertices) {
        v.position = position + v.position * Vec3f{size.x, size.y, 1.0f};
    }

    return result;
}
}

