#pragma once

#include "parsing.h"

namespace ek::flash {

struct edge_t;

edge_t& operator<<(edge_t& r, const xml_node& node);

}


