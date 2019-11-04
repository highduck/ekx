#pragma once

#include "parsing.h"

namespace ek::flash {

enum class scale_mode;
enum class solid_style_type;

enum class line_caps;
enum class line_joints;

enum class fill_type;

enum class spread_method;

struct solid_stroke;
struct gradient_entry;

struct fill_style;

struct stroke_style;

scale_mode& operator<<(scale_mode& r, const char* str);

solid_style_type& operator<<(solid_style_type& r, const char* str);

line_caps& operator<<(line_caps& r, const char* str);

line_joints& operator<<(line_joints& r, const char* str);

fill_type& operator<<(fill_type& r, const char* str);

spread_method& operator<<(spread_method& r, const char* str);

solid_stroke& operator<<(solid_stroke& solid, const xml_node& node);

gradient_entry& operator<<(gradient_entry& r, const xml_node& node);

fill_style& operator<<(fill_style& r, const xml_node& node);

stroke_style& operator<<(stroke_style& r, const xml_node& node);


}


