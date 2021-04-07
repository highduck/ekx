#pragma once

#include <ek/imaging/image.hpp>

#include <ek/math/vec.hpp>
#include <ek/math/box.hpp>
#include <ek/math/packed_color.hpp>

#include <ek/ds/Array.hpp>

namespace pugi {
class xml_node;
}

namespace ek {

enum class SpriteFilterType {
    None,
    ReFill,
    Glow,
    Shadow
};

struct SpriteFilter {
    SpriteFilterType type = SpriteFilterType::None;
    float2 blur;
    float strength = 1.0f;
    int quality = 1;
    argb32_t color = argb32_t::black;
    bool knockout = false;
    bool inner_glow = false;
    bool hide_object = false;
    float angle = 0.0f;
    float distance = 0.0f;

    // refill options
    float top = 0.0f;
    float bottom = 100.0f;
    argb32_t color_bottom = argb32_t::one;
};

struct SpriteFilterList {
    Array<SpriteFilter> list{};

    void readFromXML(const pugi::xml_node& node);

    void writeToXML(pugi::xml_node& node) const;
};

rect_i get_filtered_rect(const rect_i& rc, const Array<SpriteFilter>& filters);

Array<SpriteFilter> apply_scale(const Array<SpriteFilter>& filters, float scale);

void apply(image_t& image, const SpriteFilter& filter, const rect_i& bounds);

struct SpriteData;

void apply(const Array<SpriteFilter>& filters, SpriteData& sprite, float scale);

}

