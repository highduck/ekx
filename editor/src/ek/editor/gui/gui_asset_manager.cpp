#include <ek/editor/imgui/imgui.hpp>
#include <ek/editor/assets/editor_project.hpp>
#include <ek/editor/assets/editor_asset.hpp>
#include "editor_widgets.hpp"

namespace ek {

void gui_asset_project(editor_project_t& project) {
    ImGui::Begin("Project");
    if (ImGui::Button("Load All")) {
        project.load_all();
    }
    ImGui::SameLine();
    if (ImGui::Button("Unload All")) {
        project.unload_all();
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        project.populate();
    }
    ImGui::SameLine();
    if (ImGui::Button("Build Assets")) {
        project.build(path_t{"assets"});
    }

    for (auto* asset : project.assets) {
        if (ImGui::TreeNode(asset, "%s (%s)", asset->get_name().c_str(), asset->get_type_name().c_str())) {
            ImGui::LabelText("Declaration", "%s", asset->get_declaration_path().c_str());
            ImGui::LabelText("Resource", "%s", asset->get_resource_path().c_str());
            gui_asset_object_controls(asset);
            asset->gui();
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

}