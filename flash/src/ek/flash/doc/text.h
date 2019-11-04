#pragma once

#include <string>
#include <ek/math/vec_fwd.hpp>

namespace ek::flash {

struct text_attributes_t {
    float2 alignment{};// alignment = "left"; / center / right
    bool alias_text = false;
    float4 color{0.0f, 0.0f, 0.0f, 1.0f};
    std::string face; // face="Font 1*"
    float line_height = 20; // 20
    float line_spacing = 0; // "-14";
    float size = 32;// = "32";
    uint32_t bitmap_size = 640; // just twips size
};

struct text_run_t {
    std::string characters;
    text_attributes_t attributes;
};

}