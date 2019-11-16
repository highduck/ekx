#include "editor_assets.hpp"

#include <ek/system/working_dir.hpp>
#include <ek/system/system.hpp>
#include <scenex/asset2/asset_object.hpp>
#include <pugixml.hpp>
#include <ek/serialize/serialize.hpp>

namespace ek {

void export_all_assets(scenex::asset_manager_t& project) {
    make_dirs(project.export_path);
    output_memory_stream out{100};
    for (auto* asset : project.assets) {
        asset->export_();
        asset->export_meta(out);
    }
    std::string empty_string{};
    IO io{out};
    io(empty_string, empty_string);
    ::ek::save(out, project.export_path / "pack_meta");
}

std::string erase_sub_str(const std::string& str, const std::string& to_erase) {
    std::string result{str};
    auto pos = result.find(to_erase);
    if (pos != std::string::npos) {
        // If found then erase it from string
        result.erase(pos, to_erase.length());
    }
    return result;
}

void scan_assets_folder(scenex::asset_manager_t& project, bool load) {
    project.clear();
    std::vector<path_t> files{};
    working_dir_t::with(project.base_path, [&files, &project]() {
        files = search_files("*.xml", path_t{""});
    });
    for (auto& path : files) {
        project.add_file(path.str());
    }
    if (load) {
        project.load_all();
    }
}

bool check_xml_meta_asset(const std::string& type, const path_t& path) {
    pugi::xml_document doc{};
    if (doc.load_file(path.c_str())) {
        return doc.first_child().attribute("type").as_string() == type;
    }
    return false;
}

}