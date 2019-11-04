#pragma once

#include "static_mesh.hpp"
#include <ek/array_buffer.hpp>

namespace scenex {

mesh_data_t load_obj(const ek::array_buffer& buffer);

}

