#pragma once

#include <ek/imaging/image.hpp>

#include <ek/math/vec.hpp>
#include <ek/math/box.hpp>
#include <ek/math/packed_color.hpp>

#include <vector>

namespace pugi {
class xml_node;
}

namespace ek {

enum class filter_type {
    bypass,
    refill,
    glow,
    shadow
};

struct filter_data_t {
    filter_type type = filter_type::bypass;
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

struct FiltersDecl {
    std::vector<filter_data_t> filters;

    void readFromXML(const pugi::xml_node& node);

    void writeToXML(pugi::xml_node& node) const;
};

rect_i get_filtered_rect(const rect_i& rc, const std::vector<filter_data_t>& filters);

std::vector<filter_data_t> apply_scale(const std::vector<filter_data_t>& filters, float scale);

void apply(image_t& image, const filter_data_t& filter, const rect_i& bounds);

namespace spritepack {
struct sprite_t;
}

void apply(const std::vector<filter_data_t>& filters, spritepack::sprite_t& sprite, float scale);

}

