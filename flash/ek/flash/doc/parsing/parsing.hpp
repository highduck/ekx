#pragma once

#include <cstdint>
#include <ek/math/vec.hpp>
#include <ek/math/box.hpp>
#include <ek/math/mat3x2.hpp>
#include <ek/math/color_transform.hpp>

namespace pugi {

class xml_node;

class xml_document;

}

namespace ek::flash {

using xml_node = pugi::xml_node;
using xml_document = pugi::xml_document;

template<typename T>
T parse_xml_node(const xml_node& node) {
    T r;
    r << node;
    return r;
}


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

enum class blend_mode_t;
enum class scale_mode;
enum class solid_style_type;
enum class line_caps;
enum class line_joints;
enum class fill_type;
enum class spread_method;
enum class filter_kind_t;
enum class tween_type;
enum class rotation_direction;
enum class layer_type;
enum class element_type;
enum class symbol_type;
enum class loop_mode;

struct item_properties;
struct element_t;
struct folder_item;
struct document_info;
struct gradient_entry;
struct fill_style;
struct stroke_style;
struct filter_t;
struct motion_object_t;
struct text_attributes_t;
struct text_run_t;
struct element_t;
struct frame_t;
struct layer_t;
struct timeline_t;
struct edge_t;
struct bitmap_t;

class basic_entry;

blend_mode_t& operator<<(blend_mode_t& r, const char* str);

scale_mode& operator<<(scale_mode& r, const char* str);

solid_style_type& operator<<(solid_style_type& r, const char* str);

line_caps& operator<<(line_caps& r, const char* str);

line_joints& operator<<(line_joints& r, const char* str);

fill_type& operator<<(fill_type& r, const char* str);

spread_method& operator<<(spread_method& r, const char* str);

gradient_entry& operator<<(gradient_entry& r, const xml_node& node);

fill_style& operator<<(fill_style& r, const xml_node& node);

stroke_style& operator<<(stroke_style& r, const xml_node& node);

filter_kind_t& operator<<(filter_kind_t& r, const char* str);

filter_t& operator<<(filter_t& r, const xml_node& node);

tween_type& operator<<(tween_type& r, const char* str);

rotation_direction& operator<<(rotation_direction& r, const char* str);

text_attributes_t& operator<<(text_attributes_t& r, const xml_node& node);

text_run_t& operator<<(text_run_t& r, const xml_node& node);

layer_type& operator<<(layer_type& r, const char* str);

element_type& operator<<(element_type& r, const char* str);

symbol_type& operator<<(symbol_type& r, const char* str);

loop_mode& operator<<(loop_mode& r, const char* str);

item_properties& operator<<(item_properties& r, const xml_node& node);

folder_item& operator<<(folder_item& r, const xml_node& node);

element_t& operator<<(element_t& r, const xml_node& node);

frame_t& operator<<(frame_t& r, const xml_node& node);

layer_t& operator<<(layer_t& r, const xml_node& node);

timeline_t& operator<<(timeline_t& r, const xml_node& node);

document_info& operator<<(document_info& r, const xml_node& node);

edge_t& operator<<(edge_t& r, const xml_node& node);

bitmap_t* load_bitmap(const basic_entry& entry);

}


