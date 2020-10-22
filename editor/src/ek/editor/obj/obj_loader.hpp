#pragma once

#include <ek/scenex/3d/static_mesh.hpp>
#include <vector>
#include <cstdint>

namespace ek {

mesh_data_t load_obj(const std::vector<uint8_t>& buffer);

}

