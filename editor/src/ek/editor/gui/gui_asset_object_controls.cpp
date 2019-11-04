#include <scenex/asset2/asset_object.hpp>
#include <ek/editor/imgui/imgui.hpp>

using scenex::asset_object_t;

namespace ek {

void gui_asset_object_controls(asset_object_t* asset) {
    if (asset) {
        ImGui::PushID(asset);
        ImGui::BeginGroup();
        //ImGui::LabelText("Name", "%s", asset->name_.c_str());
        //ImGui::LabelText("Face Path", "%s", face_path_.c_str());
        if (ImGui::SmallButton("Export")) {
            asset->export_();
        }
        if (ImGui::SmallButton("Unload")) {
            asset->unload();
        }
        if (ImGui::SmallButton("Load")) {
            asset->load();
        }
        if (ImGui::SmallButton("Save")) {
            asset->save();
        }
        ImGui::EndGroup();
        ImGui::PopID();
    }
}
}