#pragma once

#include "parsing.h"

namespace ek::flash {

/* motion */
struct motion_object_t;

enum class tween_type;

enum class rotation_direction;

tween_type& operator<<(tween_type& r, const char* str);

rotation_direction& operator<<(rotation_direction& r, const char* str);

/* text */
struct text_attributes_t;

struct text_run_t;

text_attributes_t& operator<<(text_attributes_t& r, const xml_node& node);

text_run_t& operator<<(text_run_t& r, const xml_node& node);

/* element */
struct element_t;

struct frame_t;

enum class layer_type;

struct layer_t;

struct timeline_t;

enum class element_type;

enum class symbol_type;

struct item_properties;

struct element_t;

struct folder_item;

struct document_info;

layer_type& operator<<(layer_type& r, const char* str);

element_type& operator<<(element_type& r, const char* str);

symbol_type& operator<<(symbol_type& r, const char* str);

item_properties& operator<<(item_properties& r, const xml_node& node);

folder_item& operator<<(folder_item& r, const xml_node& node);

element_t& operator<<(element_t& r, const xml_node& node);

frame_t& operator<<(frame_t& r, const xml_node& node);

layer_t& operator<<(layer_t& r, const xml_node& node);

timeline_t& operator<<(timeline_t& r, const xml_node& node);

document_info& operator<<(document_info& r, const xml_node& node);

}


