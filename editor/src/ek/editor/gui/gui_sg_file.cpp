#include <ek/editor/imgui/imgui.hpp>
#include <ek/scenex/data/sg_data.hpp>

namespace ek {

void gui_sg_file_view(const sg_file* library) {
    if (library) {
        if (ImGui::TreeNode(library, "SG Library")) {
            for (const auto&[id, symbol] : library->linkages) {
                ImGui::LabelText("Symbol", "%s", id.c_str());
            }
            ImGui::TreePop();
        }
    }
}

}