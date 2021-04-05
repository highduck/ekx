#pragma once

#include "Material3D.hpp"
#include <ek/ds/Array.hpp>
#include <ek/graphics/graphics.hpp>
#include <string>

namespace ek {

struct Vertex3D {
    float3 position;
    float3 normal;
    float2 uv;
    abgr32_t color;
    abgr32_t color2;
};

struct MeshData {
    Array<Vertex3D> vertices;
    Array<uint16_t> indices;

    static MeshData createCube(const float3& position, const float3& size, argb32_t color = argb32_t::one);

    static MeshData createPlane(const float3& position, const float2& size, argb32_t color = argb32_t::one);
};

struct StaticMesh {
    graphics::Buffer vb;
    graphics::Buffer ib;
    uint32_t indices_count = 0;
    MeshData origin;

    explicit StaticMesh(const MeshData& mesh) :
            vb{
                    graphics::BufferType::VertexBuffer,
                    mesh.vertices.data(),
                    (int) (mesh.vertices.size() * sizeof(Vertex3D))
            },
            ib{
                    graphics::BufferType::IndexBuffer,
                    mesh.indices.data(),
                    (int) (mesh.indices.size() * sizeof(uint16_t))
            },
            origin{mesh} {
        indices_count = static_cast<uint32_t>(mesh.indices.size());
    }
};

struct MeshRenderer /* Component */ {
    StaticMesh* meshPtr = nullptr;
    std::string mesh;
    std::string material;
    bool castShadows = true;
    bool receiveShadows = true;
};

}

