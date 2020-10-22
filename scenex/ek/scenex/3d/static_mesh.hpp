#pragma once

#include "material_3d.hpp"
#include <vector>
#include <string>
#include <ek/graphics/vertex_decl.hpp>
#include <ek/graphics/buffer.hpp>

namespace ek {

struct mesh_data_t {
    std::vector<graphics::vertex_3d> vertices;
    std::vector<uint16_t> indices;
};

mesh_data_t create_cube(const float3& position, const float3& size, argb32_t color = argb32_t::one);

mesh_data_t create_plane(const float3& position, const float2& size, argb32_t color = argb32_t::one);

struct static_mesh_t {
    graphics::buffer_t vb;
    graphics::buffer_t ib;
    int32_t indices_count = 0;
    mesh_data_t origin;

    explicit static_mesh_t(const mesh_data_t& mesh)
            : vb{graphics::buffer_type::vertex_buffer, graphics::buffer_usage::static_buffer},
              ib{graphics::buffer_type::index_buffer, graphics::buffer_usage::static_buffer},
              origin{mesh} {
        vb.upload(mesh.vertices.data(), mesh.vertices.size() * sizeof(graphics::vertex_3d));
        ib.upload(mesh.indices.data(), mesh.indices.size() * sizeof(uint16_t));
        indices_count = mesh.indices.size();
    }
};

struct mesh_renderer_component {
    static_mesh_t* mesh_ptr = nullptr;
    std::string mesh;
    std::string material;
    bool cast_shadows = true;
    bool receive_shadows = true;
};

}
