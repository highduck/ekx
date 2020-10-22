#pragma once

#include <pugixml.hpp>
#include <vector>
#include <ek/imaging/filters.hpp>

namespace ek {

struct filters_decl_t {
    std::vector<filter_data_t> filters;
};

void from_xml(pugi::xml_node node, filter_data_t& filter);

void to_xml(pugi::xml_node node, const filter_data_t& filter);

void from_xml(pugi::xml_node node, filters_decl_t& filters);

void to_xml(pugi::xml_node node, const filters_decl_t& filters);

}

