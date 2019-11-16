#include <scenex/asset2/asset_manager.hpp>
#include <ek/editor/imgui/imgui.hpp>
#include <ek/editor/assets/editor_assets.hpp>

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
        export_all_assets(project);
    }

    for (auto* asset : project.assets) {
        asset->gui();
    }
    ImGui::End();
}

}