#include "project_config.h"

#include <pugixml.hpp>
#include <ek/fs/path.hpp>

namespace ekc {

void parse_config_html_section(html_config_t& html, const pugi::xml_node& node) {
    html.google_analytics_property_id = node.attribute("google_analytics_property_id").as_string("");
    html.background_color = node.attribute("background_color").as_string("black");
    html.text_color = node.attribute("text_color").as_string("white");
    html.url = node.attribute("url").as_string("https://eliasku.github.io");
    html.image_url = node.attribute("image_url").as_string("https://eliasku.github.io");
    html.deploy_dir = ek::path_t{node.attribute("deploy_dir").as_string("")};
}

void parse_asset_list(assets_config_t& assets, const pugi::xml_node& node) {
    for (auto& asset_node : node.children()) {
        asset_entry_config_t entry;
        entry.name = asset_node.attribute("name").as_string();
        entry.path = ek::path_t{asset_node.attribute("path").as_string()};
        entry.type = asset_node.attribute("type").as_string();
        entry.input = assets.input;
        entry.output = assets.output;
        assets.list.push_back(std::move(entry));
    }
}

void parse_project_config(project_config_t& out_config, const char* path) {
    pugi::xml_document doc;
    doc.load_file(path);
    auto node = doc.child("project");
    out_config.name = node.attribute("name").as_string("unnamed");
    out_config.binary_name = node.attribute("binary_name").as_string(out_config.name.c_str());
    out_config.cmake_target = node.attribute("cmake_target").as_string(out_config.name.c_str());
    out_config.build_dir = node.attribute("build_dir").as_string("build");
    out_config.title = node.attribute("title").as_string("");
    out_config.desc = node.attribute("desc").as_string("");
    out_config.version_name = node.attribute("version_name").as_string("0.0.1");
    out_config.version_code = node.attribute("version_code").as_string("1");
    out_config.build_number = node.attribute("build_number").as_string("1");
    out_config.orientation = node.attribute("orientation").as_string("portrait");
    out_config.pwa_url = node.attribute("pwa_url").as_string("");

    parse_config_html_section(out_config.html, node.child("html"));

    auto assets_node = node.child("assets");
    out_config.assets.builtin = assets_node.attribute("builtin").as_bool(false);
    out_config.assets.input = ek::path_t{assets_node.attribute("input").as_string("")};
    out_config.assets.output = ek::path_t{assets_node.attribute("output").as_string("")};
    if (out_config.assets.builtin) {
        out_config.assets.input = out_config.path_ekc / "resources";
    }
    parse_asset_list(out_config.assets, assets_node);

    auto android_node = node.child("android");
    if (!android_node.empty()) {
        {
            const auto application_node = android_node.child("application");
            if (!application_node.empty()) {
                out_config.android.application_id = application_node.attribute("id").as_string();
                out_config.android.package_id = application_node.attribute("package").as_string();
            }
        }

        {
            const auto admob_node = android_node.child("admob");
            if (!admob_node.empty()) {
                out_config.android.admob_app_id = admob_node.attribute("app_id").as_string();
            }
        }

        {
            const auto game_services_node = android_node.child("game_services");
            if (!game_services_node.empty()) {
                out_config.android.game_services_id = game_services_node.attribute("id").as_string();
            }
        }

        {
            const auto keystore_node = android_node.child("keystore");
            if (!keystore_node.empty()) {
                out_config.android.keystore.store_keystore = ek::path_t{keystore_node.child("store_keystore").text().as_string()};
                out_config.android.keystore.store_password = keystore_node.child("store_password").text().as_string();
                out_config.android.keystore.key_alias = keystore_node.child("key_alias").text().as_string();
                out_config.android.keystore.key_password = keystore_node.child("key_password").text().as_string();
            }
        }
    }

    auto ios_node = node.child("ios");
    if (!ios_node.empty()) {
        const auto application_node = ios_node.child("application");
        if (!application_node.empty()) {
            out_config.ios.application_id = application_node.attribute("id").as_string();
        }
    }
}

void create_project_config(project_config_t& out_config) {
    init_project_config_defaults(out_config);
    auto builtin_config_path = out_config.path_ekc / "resources" / "ek.xml";
    parse_project_config(out_config, builtin_config_path.c_str());
    parse_project_config(out_config, "ek.xml");
}

}