#pragma once

#include <scenex/3d/static_mesh.hpp>
#include <vector>
#include <cstdint>

namespace ek {

scenex::mesh_data_t load_obj(const std::vector<uint8_t>& buffer);

}

