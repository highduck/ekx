#pragma once

#include <string>
#include <memory>
#include <ek/array_buffer.hpp>
#include <ek/fs/path.hpp>

namespace ek::flash {

struct bitmap_t {
    path_t path;
    int width = 0;
    int height = 0;
    int bpp = 4;
    bool alpha = true;
    std::unique_ptr<array_buffer> data;
};

}


