#pragma once

#include <scenex/3d/static_mesh.hpp>
#include <ek/array_buffer.hpp>

namespace ek {

scenex::mesh_data_t load_obj(const ek::array_buffer& buffer);

}

