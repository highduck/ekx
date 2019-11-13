#pragma once

#include <scenex/3d/static_mesh.hpp>
#include <graphics/vertex_decl.hpp>
#include <ek/serialize/serialize.hpp>

namespace ek {

template<>
struct declared_as_pod_type<vertex_3d> : public std::true_type {
};

}

namespace scenex {

struct model_data_t {
    mesh_data_t mesh;

    template<typename S>
    void serialize(ek::IO<S>& io) {
        io(mesh.vertices, mesh.indices);
    }
};

}

