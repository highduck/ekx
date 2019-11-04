#include "editor_widgets.hpp"

#include <scenex/asset2/asset_manager.hpp>
#include <scenex/asset2/asset_object.hpp>
#include <ek/editor/imgui/imgui.hpp>
#include <ek/fs/system.hpp>
#include <ek/fs/path.hpp>
#include <ek/fs/working_dir.hpp>
#include <ek/serialize/serialize.hpp>

namespace ek {

void gui_asset_project(scenex::asset_manager_t& project) {
    ImGui::Begin("Project");
    if (ImGui::Button("Load All")) {
        project.load_all();
    }
    if (ImGui::Button("Unload All")) {
        project.unload_all();
    }
    if (ImGui::Button("Refresh")) {
        scan_assets_folder(project);
    }
    if (ImGui::Button("Export All")) {
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

    for (auto* asset : project.assets) {
        asset->gui();
    }
    ImGui::End();
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

void scan_assets_folder(scenex::asset_manager_t& project) {
    project.clear();
    std::vector<path_t> files{};
    working_dir_t::with(project.base_path, [&files, &project]() {
        files = search_files("*.xml", path_t{""});
    });
    for (auto& path : files) {
        project.add_file(path.str());
    }
    project.load_all();
}

}