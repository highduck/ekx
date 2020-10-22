#pragma once

#include <ek/scenex/3d/static_mesh.hpp>
#include <ek/graphics/vertex_decl.hpp>
#include <ek/serialize/serialize.hpp>

namespace ek {

template<>
struct declared_as_pod_type<graphics::vertex_3d> : public std::true_type {
};

struct model_data_t {
    mesh_data_t mesh;

    template<typename S>
    void serialize(IO <S>& io) {
        io(mesh.vertices, mesh.indices);
    }
};

}

