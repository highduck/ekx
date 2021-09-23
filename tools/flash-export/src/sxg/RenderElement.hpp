#pragma once

namespace ek {
struct SpriteData;
}

namespace ek::xfl {

class Doc;

struct Element;

struct RenderElementOptions {
    float scale = 1.0f;
    int width = 0;
    int height = 0;
    bool alpha = true;
    bool trim = false;
};

SpriteData renderElement(const Doc& doc, const Element& el, const RenderElementOptions& options);

}
