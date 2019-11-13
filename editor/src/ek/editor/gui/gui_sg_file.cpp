#include <ek/editor/imgui/imgui.hpp>
#include <scenex/data/sg_data.h>

namespace ek {

void gui_sg_file_view(const scenex::sg_file* library) {
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