#include "parsing.hpp"

#include <ek/math/mat3x2.hpp>
#include <pugixml.hpp>
#include <cstring>
#include <cstdlib>

namespace ek::flash {

bool equals(const char* a, const char* b) {
    return a && strcmp(a, b) == 0;
}

uint32_t parse_css_color(const char* str) {
    uint32_t col = 0x0;
    if (str) {
        if (*str == '#') {
            ++str;
        }
        col = strtoul(str, nullptr, 16);
    }
    return col;
}

rect_f read_rect(const xml_node& node) {
    return {
            node.attribute("left").as_float(),
            node.attribute("top").as_float(),
            node.attribute("width").as_float(),
            node.attribute("height").as_float()
    };
}

rect_f read_rect_bounds(const xml_node& node, const std::array<const char*, 4>& nn) {
    return min_max_box<float, 2>(
            {node.attribute(nn[0]).as_float(), node.attribute(nn[1]).as_float()},
            {node.attribute(nn[2]).as_float(), node.attribute(nn[3]).as_float()}
    );
}

rect_f read_scale_grid(const xml_node& node) {
    return read_rect_bounds(node, {
            "scaleGridLeft",
            "scaleGridTop",
            "scaleGridRight",
            "scaleGridBottom"
    });
}

float2 read_point(const xml_node& node) {
    return {node.attribute("x").as_float(), node.attribute("y").as_float()};
}

float2 read_transformation_point(const xml_node& node) {
    return read_point(node.child("transformationPoint").child("Point"));
}

matrix_2d& operator<<(matrix_2d& r, const xml_node& node) {
    const auto& m = node.child("matrix").child("Matrix");
    r.a = m.attribute("a").as_float(1.0f);
    r.b = m.attribute("b").as_float();
    r.c = m.attribute("c").as_float();
    r.d = m.attribute("d").as_float(1.0f);
    r.tx = m.attribute("tx").as_float();
    r.ty = m.attribute("ty").as_float();
    return r;
}

color_transform_f& operator<<(color_transform_f& color, const xml_node& node) {
    const auto& ct = node.child("color").child("Color");

    color.multiplier.x = ct.attribute("redMultiplier").as_float(1.0f);
    color.multiplier.y = ct.attribute("greenMultiplier").as_float(1.0f);
    color.multiplier.z = ct.attribute("blueMultiplier").as_float(1.0f);
    color.multiplier.w = ct.attribute("alphaMultiplier").as_float(1.0f);

    color.offset.x = ct.attribute("redOffset").as_float() / 255.0f;
    color.offset.y = ct.attribute("greenOffset").as_float() / 255.0f;
    color.offset.z = ct.attribute("blueOffset").as_float() / 255.0f;
    color.offset.w = ct.attribute("alphaOffset").as_float() / 255.0f;

    auto tint_multiplier = ct.attribute("tintMultiplier").as_float();
    auto tint_color = parse_css_color(ct.attribute("tintColor").value());
    if (tint_multiplier > 0.0f) {
        color.tint(tint_color, tint_multiplier);
    }

    // default: 0, values: -1 ... 1
    const auto br = math::clamp(ct.attribute("brightness").as_float(0.0f), -1.0f, 1.0f);
    if (br < 0.0f) {
        color.multiplier.x =
        color.multiplier.y =
        color.multiplier.z = 1.0f + br;
    } else if (br > 0.0f) {
        color.offset.x =
        color.offset.y =
        color.offset.z = br;
    }

    return color;
}

float4 read_color(const xml_node& node, const char* color_tag, const char* alpha_tag) {
    const auto c = parse_css_color(node.attribute(color_tag).value());
    return {
            static_cast<float>((c >> 16u) & 0xFFu) / 255.0f,
            static_cast<float>((c >> 8u) & 0xFFu) / 255.0f,
            static_cast<float>(c & 0xFFu) / 255.0f,
            node.attribute(alpha_tag).as_float(1.0f)
    };
}

}