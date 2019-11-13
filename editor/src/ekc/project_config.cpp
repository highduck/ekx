#include "project_config.hpp"

#include <pugixml.hpp>
#include <ek/fs/path.hpp>
#include <ek/system/system.hpp>
#include <ek/logger.hpp>
#include <ek/system/working_dir.hpp>
#include <ek/utility/strings.hpp>

using ek::path_t;

namespace ekc {

void init_project_path(project_path_t& path) {
    const auto* ekx_root = std::getenv("EKX_ROOT");
    if (!ekx_root) {
        EK_ERROR << "Please define EKX_ROOT environment variable. Abort.";
        abort();
    }

    path.ekx = path_t{ekx_root};

    if (!ek::is_dir(path.ekx)) {
        EK_ERROR << "EKX_ROOT is not a directory: " << path.ekx;
        abort();
    }

    path.emsdk = path_t{"/Users/ilyak/dev/emsdk"};
    if (!ek::is_dir(path.emsdk)) {
        EK_WARN << "Emscripten SDK dir is not found: " << path.ekx;
        EK_WARN << "Web Target is not available";
        path.emsdk_toolchain = path.emsdk /
                               "upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake";
        if (!ek::is_file(path.emsdk_toolchain)) {
            EK_WARN << "Emscripten SDK toolchain is not found: " << path.ekx;
            EK_WARN << "Web Target is not available";
        }
    }

    path.current_project = path.project = path_t{ek::current_working_directory()};
}

bool check_current_target(pugi::xml_node node, const project_config_t& config) {
    return config.current_target == node.attribute("target").as_string(config.current_target.c_str());
}

void include_project(const std::string& module_name, project_config_t& config);

path_t process_path(const project_config_t& config, const std::string& str) {
    auto res = ek::replace(str, "${EKX}", config.path.ekx.str());
    res = ek::replace(res, "${CURRENT_PROJECT_DIR}", (config.path.current_project).str());
    res = ek::replace(res, "${PROJECT_DIR}", (config.path.project).str());
    res = ek::replace(res, "${OUTPUT}", (config.path.project / config.build_dir).str());
    return path_t{res};
}

void populate_config(pugi::xml_node root, project_config_t& config) {
    using ek::path_t;
    using ek::is_file;

    for (auto& module_ref : root.children("module")) {
        if (check_current_target(module_ref, config)) {
            include_project(module_ref.attribute("name").as_string(), config);
        }
    }

    for (auto template_node : root.children("template")) {
        if (check_current_target(template_node, config)) {
            module_template_t tpl{};
            tpl.source = process_path(config, template_node.attribute("from").as_string());
            tpl.dest = process_path(config, template_node.attribute("to").as_string());
            config.templates.push_back(tpl);
        }
    }
}

void include_project(const std::string& module_name, project_config_t& config) {
    EK_DEBUG << "include project: " << module_name;
    if (!module_name.empty()) {
        path_t old_path = config.path.current_project;
        config.path.current_project = config.path.ekx / module_name;
        path_t module_path = config.path.ekx / module_name / "ek.xml";
        pugi::xml_document doc;
        if (doc.load_file(module_path.c_str())) {
            populate_config(doc.child("project"), config);
        } else {
            EK_WARN << "Error read XML: " << module_path;
        }
        config.path.current_project = old_path;
    }
}

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
    out_config.assets.input = ek::path_t{assets_node.attribute("input").as_string("")};
    out_config.assets.output = ek::path_t{assets_node.attribute("output").as_string("")};
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
                out_config.android.keystore.store_keystore = ek::path_t{
                        keystore_node.child("store_keystore").text().as_string()};
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

    include_project("ek", out_config);
    populate_config(node, out_config);
}

void create_project_config(project_config_t& out_config) {
    init_project_path(out_config.path);
    parse_project_config(out_config, "ek.xml");
}

}