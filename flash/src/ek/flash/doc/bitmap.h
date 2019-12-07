#pragma once

#include <vector>
#include <cstdint>
#include <ek/util/path.hpp>

namespace ek::flash {

struct bitmap_t {
    path_t path;
    int width = 0;
    int height = 0;
    int bpp = 4;
    bool alpha = true;
    std::vector<uint8_t> data;
};

}


