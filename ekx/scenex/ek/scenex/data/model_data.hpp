#pragma once

#include <ek/scenex/3d/StaticMesh.hpp>
#include <ek/serialize/serialize.hpp>

namespace ek {

template<>
struct declared_as_pod_type<Vertex3D> : public std::true_type {
};

struct model_data_t {
    MeshData mesh;

    template<typename S>
    void serialize(IO <S>& io) {
        io(mesh.vertices, mesh.indices);
    }
};

}

