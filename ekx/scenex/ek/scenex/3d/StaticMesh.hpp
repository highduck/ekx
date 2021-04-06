#pragma once

#include "Material3D.hpp"
#include <ek/ds/Array.hpp>
#include <ek/graphics/graphics.hpp>
#include <ek/scenex/data/Model3D.hpp>
#include <string>

namespace ek {

struct StaticMesh {
    graphics::Buffer vb;
    graphics::Buffer ib;
    uint32_t indices_count = 0;
    Model3D origin;

    explicit StaticMesh(const Model3D& model) :
            vb{
                    graphics::BufferType::VertexBuffer,
                    model.vertices.data(),
                    (int) (model.vertices.size() * sizeof(ModelVertex3D))
            },
            ib{
                    graphics::BufferType::IndexBuffer,
                    model.indices.data(),
                    (int) (model.indices.size() * sizeof(uint16_t))
            },
            origin{model} {
        indices_count = static_cast<uint32_t>(model.indices.size());
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

