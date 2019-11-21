#include <scenex/asset2/asset_manager.hpp>
#include <ek/editor/imgui/imgui.hpp>
#include <ek/editor/assets/editor_assets.hpp>
#include <ek/editor/assets/editor_asset.hpp>
#include "editor_widgets.hpp"

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

    for (auto* base_asset : project.assets) {
        auto* asset = dynamic_cast<editor_asset_t*>(base_asset);
        if (asset) {
            if (ImGui::TreeNode(asset, "%s (%s)", asset->get_name().c_str(), asset->get_type_name().c_str())) {
                ImGui::LabelText("Declaration", "%s", asset->get_path().c_str());
                ImGui::LabelText("Resource", "%s", asset->get_resource_path().c_str());
                gui_asset_object_controls(asset);
                asset->gui();
                ImGui::TreePop();
            }
        }
    }
    ImGui::End();
}

}