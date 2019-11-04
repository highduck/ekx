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

struct template_vars_t;

struct asset_entry_config_t {
    std::string name;
    ek::path_t path;
    ek::path_t input;
    ek::path_t output;
    std::string type;
};

struct assets_config_t {
    ek::path_t input;
    ek::path_t output;
    std::vector<asset_entry_config_t> list;
};

struct html_config_t {
    std::string background_color;
    std::string text_color;
    std::string google_analytics_property_id;
    std::string url;
    std::string image_url;
    ek::path_t deploy_dir;
};

struct android_keystore_t {
    // filename for mykey.keystore
    ek::path_t store_keystore;
    std::string store_password;
    std::string key_alias;
    std::string key_password;
};

struct android_config_t {
    std::string application_id;
    std::string package_id;
    std::string admob_app_id;
    std::string game_services_id;
    android_keystore_t keystore;
};

struct ios_config_t {
    std::string application_id;
};

struct project_config_t {
    ek::path_t path_ekx;
    ek::path_t path_emsdk_toolchain;
    ek::path_t path;

    std::string binary_name;
    std::string cmake_target;
    std::string build_dir;
    std::string name;
    std::string title;
    std::string desc;
    std::string version_name;
    std::string version_code;
    std::string build_number;

    // portrait / landscape
    std::string orientation;

    std::string pwa_url;

    android_config_t android;
    ios_config_t ios;

    html_config_t html;
    assets_config_t assets;
};

void init_project_config_defaults(project_config_t& dest);

void create_project_config(project_config_t& dest);

void fill_template_vars(const project_config_t& config, template_vars_t& vars);

}


