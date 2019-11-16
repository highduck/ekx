#pragma once

#include <vector>

namespace ek::flash {

struct edge_t {
    std::vector<char> commands;
    std::vector<double> values;
    int fill_style_0 = 0;
    int fill_style_1 = 0;
    int stroke_style = 0;
};

}


