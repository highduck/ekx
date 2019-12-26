#pragma once

#include <string>
#include <ek/scenex/2d/font.hpp>
#include <ek/scenex/data/sg_data.hpp>

namespace ek {

std::string to_json_str(const font_data_t& font);
std::string to_json_str(const sg_file& ani);

}

