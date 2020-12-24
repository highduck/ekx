#pragma once

#include <ek/scenex/3d/StaticMesh.hpp>
#include <vector>
#include <cstdint>

namespace ek {

MeshData load_obj(const std::vector<uint8_t>& buffer);

}

