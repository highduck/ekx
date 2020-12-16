#pragma once

#include <ek/builders/MultiResAtlas.hpp>

namespace ek::xfl {

class Doc;

struct Element;

struct renderer_options_t {
    float scale = 1.0f;
    int width = 0;
    int height = 0;
    bool alpha = true;
    bool trim = false;
};

SpriteData render(const Doc& doc, const Element& el, const renderer_options_t& options);

}
