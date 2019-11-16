#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <ek/math/color_transform.hpp>
#include <ek/math/vec_fwd.hpp>
#include <ek/math/matrix_fwd.hpp>
#include <ek/math/box.hpp>
#include <ek/math/bounds_builder.hpp>

#include "parsing.h"

namespace ek::flash {

bool equals(const char* a, const char* b);

uint32_t parse_css_color(const char* str);

float4 read_color(const xml_node& node, const char* color_tag = "color", const char* alpha_tag = "alpha");

rect_f read_rect(const xml_node& node);

rect_f read_rect_bounds(const xml_node& node, const std::array<const char*, 4>& nn);

rect_f read_scale_grid(const xml_node& node);

float2 read_point(const xml_node& node);

float2 read_transformation_point(const xml_node& node);

matrix_2d& operator<<(matrix_2d& r, const xml_node& node);

color_transform_f& operator<<(color_transform_f& color, const xml_node& node);

}


