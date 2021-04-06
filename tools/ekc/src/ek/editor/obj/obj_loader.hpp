#pragma once

#include <ek/scenex/data/Model3D.hpp>
#include <vector>
#include <cstdint>

namespace ek {

Model3D load_obj(const std::vector<uint8_t>& buffer);

}

