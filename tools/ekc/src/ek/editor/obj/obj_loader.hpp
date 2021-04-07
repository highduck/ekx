#pragma once

#include <ek/scenex/data/Model3D.hpp>
#include <ek/ds/Array.hpp>
#include <cstdint>

namespace ek {

Model3D load_obj(const Array<uint8_t>& buffer);

}

