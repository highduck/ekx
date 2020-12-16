#pragma once

#include <ek/builders/MultiResAtlas.hpp>

namespace ek::xfl {

class Doc;

struct element_t;

struct renderer_options_t {
    float scale = 1.0f;
    int width = 0;
    int height = 0;
    bool alpha = true;
    bool trim = false;
};

SpriteData render(const Doc& doc, const element_t& el, const renderer_options_t& options);

}
