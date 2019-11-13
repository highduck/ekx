#include "generate_project_common.hpp"
#include "project_config.hpp"

#include <ek/system/working_dir.hpp>
#include <ek/system/system.hpp>
#include <ek/utility/strings.hpp>
#include <ek/logger.hpp>

#include <fmt/core.h>
#include <pugixml.hpp>

#include <string>

using std::string;
using std::vector;

using namespace ek;

namespace ekc {

void open_android_project(path_t path) {
    execute("open -a /Applications/Android\\ Studio.app \"" + path.str() + "\"");
}

void copy_google_services_config_android() {
    path_t config_file{"google-services.json"};
    path_t config_path = "../.." / config_file;
    if (is_file(config_path)) {
        copy_file(config_path, "app" / config_file);
    } else {
        EK_WARN << "missing " << config_file.c_str();
    }
}

void copy_keystore(const path_t& keystore_file) {
    path_t src = "../.." / keystore_file;
    if (is_file(src)) {
        copy_file(src, "app" / keystore_file);
    } else {
        EK_WARN << "missing " << keystore_file.c_str();
    }
}

void mod_main_class(const string& app_package_java) {
    path_t template_main_activity_java{
            "app/src/main/java/com/eliasku/template_android/MainActivity.java"};
    path_t java_package_path{ek::replace(app_package_java, ".", "/")};

    auto text = read_text(template_main_activity_java);
    text = ek::replace(text,
                       "package com.eliasku.template_android;",
                       fmt::format("package {};", app_package_java));
    remove_dir_rec("app/src/main/java/com");
    const path_t main_activity_path = "app/src/main/java" / java_package_path;

    make_dirs(main_activity_path);
    save(text, main_activity_path / "MainActivity.java");
}

void mod_android_manifest(const project_config_t& config) {
    string orientation = "sensorPortrait";
    if (config.orientation == "landscape") {
        orientation = "sensorLandscape";
    } else if (config.orientation == "portrait") {
        orientation = "sensorPortrait";
    } else {
        EK_WARN << "unknown orientation: " << config.orientation;
    }

    replace_in_file(path_t{"app/src/main/AndroidManifest.xml"}, {
            {"com.eliasku.template_android", config.android.package_id},
            {R"(screenOrientation="sensorPortrait")",
                                             fmt::format(R"(screenOrientation="{}")", orientation)}
    });
}

void mod_strings(const project_config_t& config) {
    using std::string;
    using namespace pugi;

    string res_strings_path = "app/src/main/res/values/strings.xml";

    xml_document doc;
    doc.load_file(res_strings_path.c_str());
    for (auto& child : doc.root().first_child().children("string")) {
        if (strcmp(child.attribute("name").value(), "app_name") == 0) {
            child.text().set(config.title.c_str());
        } else if (strcmp(child.attribute("name").value(), "package_name") == 0) {
            child.text().set(config.android.application_id.c_str());
        } else if (strcmp(child.attribute("name").value(), "gs_app_id") == 0) {
            child.text().set(config.android.game_services_id.c_str());
        } else if (strcmp(child.attribute("name").value(), "admob_app_id") == 0) {
            child.text().set(config.android.admob_app_id.c_str());
        }
    }

    doc.save_file(res_strings_path.c_str());
}

void append(std::vector<path_t>& dest, const std::vector<path_t>& src) {
    dest.insert(dest.end(), src.begin(), src.end());
}

std::vector<path_t> collect_source_files(const path_t& path) {
    std::vector<path_t> src_files;
    append(src_files, search_files("*.h", path));
    append(src_files, search_files("*.hpp", path));
    append(src_files, search_files("*.cpp", path));
    return src_files;
}

std::vector<string> to_string_vector(const vector<path_t>& path_list) {
    std::vector<string> result{};
    result.reserve(path_list.size());
    for (auto& path : path_list) {
        result.emplace_back(path.str());
    }
    return result;
}

void mod_cmake_lists(const project_config_t& config) {
    using std::string;
    using std::vector;
    path_t cmake_path{"app/CMakeLists.txt"};
    vector<path_t> search_list{
            path_t{"../../src"},
            config.path.ekx / "ecxx/src",
            config.path.ekx / "core/src",
            config.path.ekx / "ek/platforms/android",
            config.path.ekx / "ek/src",
            config.path.ekx / "scenex/src"
    };

    vector<path_t> src_files;
    append(src_files, collect_source_files(path_t{"../../src"}));
    append(src_files, collect_source_files(config.path.ekx / "ecxx/src"));
    append(src_files, collect_source_files(config.path.ekx / "core/src"));
    append(src_files, collect_source_files(config.path.ekx / "ek/src"));
    append(src_files, collect_source_files(config.path.ekx / "ek/platforms/android"));
    append(src_files, collect_source_files(config.path.ekx / "scenex/src"));

    for (auto& it : search_list) {
        if (!it.empty() && it.str()[0] != '/') {
            it = ".." / it;
        }
    }

    for (auto& it : src_files) {
        if (!it.empty() && it.str()[0] != '/') {
            it = ".." / it;
        }
    }

    replace_in_file(cmake_path, {
            {"#-SOURCES-#",      ek::join(to_string_vector(src_files), "\n\t\t")},
            {"#-SEARCH_ROOTS-#", ek::join(to_string_vector(search_list), "\n\t\t")}
    });
}

void create_android_project(const project_config_t& config) {
    //remove_module_cache();
    update_module_cache();

    const string platform_target = "android";
    const string platform_proj_name = config.name + "-" + platform_target;
    const path_t dest_dir{"projects"};
    const path_t dest_path = dest_dir / platform_proj_name;

    if (is_dir(dest_path)) {
        printf("Remove old project: %s \n", dest_path.c_str());
        remove_dir_rec(dest_path);
        assert(!is_dir(dest_path));
    }

    copy_tree(config.path.ekx / ("ek/templates/template-" + platform_target), dest_path);
    const path_t base_path{"../.."};
    const path_t ek_path = config.path.ekx / "ek";

    working_dir_t::with(dest_path, [&]() {


        std::vector<std::string> source_sets{};
        source_sets.emplace_back(
                fmt::format("main.java.srcDirs += '{}'",
                            (ek_path / "platforms/android/java").str())
        );
        source_sets.emplace_back("main.assets.srcDirs += 'src/main/assets'");

        replace_in_file(path_t{"app/build.gradle"}, {
                {"com.eliasku.template_android", config.android.application_id},
                {"versionCode 1 // AUTO",        "versionCode " + config.version_code + " // AUTO"},
                {R"(versionName "1.0" // AUTO)", R"(versionName ")" + config.version_name + R"(" // AUTO)"},
                {R"(// TEMPLATE_SOURCE_SETS)",   ek::join(source_sets, "\n\t\t")},
                {R"(KEY_ALIAS)",                 config.android.keystore.key_alias},
                {R"(KEY_PASSWORD)",              config.android.keystore.key_password},
                {R"(store.keystore)",            config.android.keystore.store_keystore.str()},
                {R"(STORE_PASSWORD)",            config.android.keystore.store_password}
        });

        make_dirs("app/src/main/assets");
        copy_tree(base_path / config.assets.output, path_t{"app/src/main/assets/assets"});

        copy_tree(base_path / "generated/android/res", path_t{"app/src/main/res"});

        mod_main_class(config.android.package_id);
        mod_android_manifest(config);
        mod_strings(config);
        mod_cmake_lists(config);
        copy_google_services_config_android();
        copy_keystore(config.android.keystore.store_keystore);
    });

    open_android_project(dest_path);
}

}