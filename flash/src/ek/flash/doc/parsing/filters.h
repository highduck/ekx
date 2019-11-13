#pragma once

#include "parsing.h"

namespace ek::flash {

enum class filter_kind_t;

struct filter_t;

filter_kind_t& operator<<(filter_kind_t& r, const char* str);

filter_t& operator<<(filter_t& r, const xml_node& node);

}