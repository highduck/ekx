#include <ek/editor/assets/editor_asset.hpp>
#include <ek/editor/imgui/imgui.hpp>

namespace ek {

void gui_asset_object_controls(editor_asset_t* asset) {
    if (asset) {
        ImGui::PushID(asset);
        //ImGui::LabelText("Name", "%s", asset->name_.c_str());
        //ImGui::LabelText("Face Path", "%s", face_path_.c_str());
        if (ImGui::SmallButton("Unload")) {
            asset->unload();
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Load")) {
            asset->load();
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Save")) {
            asset->save();
        }
        ImGui::PopID();
    }
}
}