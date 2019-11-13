#pragma once

#include <string>
#include <vector>
#include <optional>
#include <fonts/export_font.h>
#include <fonts/font_declaration.hpp>
#include <fonts/filters_declaration.hpp>
#include <ek/spritepack/atlas_declaration.hpp>
#include <ek/fs/path.hpp>

namespace pugi {

class xml_node;

}

namespace ekc {

struct project_path_t {
    ek::path_t ekx;
    ek::path_t emsdk;
    ek::path_t emsdk_toolchain;
    ek::path_t project;
    ek::path_t current_project;
};

void init_project_path(project_path_t& path);

struct marketing_asset_t {
    std::string name{"res"};
    ek::path_t input{"assets"};
};

struct project_config_t {
    project_path_t path;
    marketing_asset_t marketing;
};

void create_project_config(project_config_t& dest);

}


