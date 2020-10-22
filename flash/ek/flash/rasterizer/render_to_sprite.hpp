#pragma once

#include <ek/spritepack/sprite_data.hpp>

namespace ek::flash {

class flash_doc;

struct element_t;

struct renderer_options_t {
    float scale = 1.0f;
    int width = 0;
    int height = 0;
    bool alpha = true;
    bool trim = false;
};

spritepack::sprite_t render(const flash_doc& doc, const element_t& el, const renderer_options_t& options);

}
