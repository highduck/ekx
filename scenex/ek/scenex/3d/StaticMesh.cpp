#include "StaticMesh.hpp"

namespace ek {

MeshData MeshData::createCube(const float3& position, const float3& size, argb32_t color) {
    MeshData result;

    // 6 sides
    // 6 indices per size
    // 2 faces per size
    result.vertices.resize(6 * 4);
    result.indices.resize(6 * 2 * 3);

    float3 n;
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

    result.indices = {
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

    for (auto& v : result.vertices) {
        v.position = position + v.position * size;
    }

    return result;
}

MeshData MeshData::createPlane(const float3& position, const float2& size, argb32_t color) {
    MeshData result;
    result.vertices.resize(4);
    result.indices.resize(2 * 3);

    float3 n;
    const float u = 0.5f;
    const auto color1 = color.abgr();
    const abgr32_t color2 = 0x0;

    n = {0, 0, 1};
    result.vertices[0] = {{-u, -u, 0.0f}, n, {0, 0}, color1, color2};
    result.vertices[1] = {{u, -u, 0.0f}, n, {1, 0}, color1, color2};
    result.vertices[2] = {{u, u, 0.0f}, n, {1, 1}, color1, color2};
    result.vertices[3] = {{-u, u, 0.0f}, n, {0, 1}, color1, color2};

    result.indices = {
            0, 1, 2,
            2, 3, 0
    };

    for (auto& v : result.vertices) {
        v.position = position + v.position * float3{size.x, size.y, 1.0f};
    }

    return result;
}

}